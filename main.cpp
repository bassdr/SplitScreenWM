#include <iostream>
#include <deque>
#include <xcb/xcb.h>
#include <xcb/xcb_event.h>

int main(int const argc, char const* const* argv)
{
  auto connection(xcb_connect(nullptr, nullptr));

  if(int const error = xcb_connection_has_error(connection))
  {
    std::cerr << "Failed to connect to X: ";

    switch(error)
    {
      case XCB_CONN_ERROR:
        std::cerr << "socket errors, pipe errors or other stream errors.";
        break;

      case XCB_CONN_CLOSED_EXT_NOTSUPPORTED:
        std::cerr << "extension not supported.";
        break;

      case XCB_CONN_CLOSED_MEM_INSUFFICIENT:
        std::cerr << "memory not available.";
        break;

      case XCB_CONN_CLOSED_REQ_LEN_EXCEED:
        std::cerr << "exceeded request length that server accepts.";
        break;

      case XCB_CONN_CLOSED_PARSE_ERR:
        std::cerr << "error during parsing display string.";
        break;

      case XCB_CONN_CLOSED_INVALID_SCREEN:
        std::cerr << "the server does not have a screen matching the display.";
        break;

      default:
        std::cerr << "unknown error code " << error;
        break;
    }
    std::cerr << std::endl;

    return error;
  }

  auto const setup(xcb_get_setup(connection));
  std::deque<xcb_void_cookie_t> rootWindowCookies;

  for (auto rootsIt(xcb_setup_roots_iterator(setup));
       true;
       xcb_screen_next(&rootsIt))
  {
    auto const rootWindow(rootsIt.data->root);

    uint32_t static const eventMask[]
    {
      XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY | XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT
    };

    rootWindowCookies.emplace_back(xcb_change_window_attributes_checked(
      connection,
      rootWindow,
      XCB_CW_EVENT_MASK,
      eventMask));

    if(rootsIt.rem <= 0)
    {
      break;
    }
  }

  bool registered(false);
  for(auto const rootWindowCookie : rootWindowCookies)
  {
    if(auto const error = xcb_request_check(connection, rootWindowCookie))
    {
      std::cerr << "Cannot subscribe to root window manager events ("
                << error->sequence << " of " << rootWindowCookies.size() <<  "): "
                << xcb_event_get_label(error->response_type) << ": "
                << xcb_event_get_error_label(error->error_code) << std::endl;
    }
    else registered = true;
  }

  while(registered)
  {
    if(auto const event = xcb_poll_for_event(connection))
    {
      std::cout << xcb_event_get_label(event->response_type) << std::endl;
    }
  }

  xcb_disconnect(connection);

  return 0;
}