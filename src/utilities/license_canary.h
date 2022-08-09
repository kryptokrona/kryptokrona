// Copyright (c) 2018, The TurtleCoin Developers
// 
// Please see the included LICENSE file for more information.

#include <string>

constexpr std::string_view canary = "TurtleCoin";

constexpr std::string_view canaryObfusticated = "\u0054\u0075\u0072\u0074\u006c\u0065\u0043\u006f\u0069\u006e";

/* Compare the canary with kryptokrona. Done as unicode to not be caught
   by find and replace. If find and replace occured, the canary will no longer
   match. Then we can warn them that they have probably replaced license headers,
   and halt compilation till it's fixed. */
static_assert(canary == canaryObfusticated, 
    "\n\n\u0057\u0061\u0069\u0074\u0020\u0061\u0020\u006d\u0069\u006e\u0075\u0074"
    "\u0065\u002c\u0020\u0068\u006f\u006c\u0064\u0020\u0075\u0070\u002c\u0020\u006c"
    "\u0065\u0074\u0027\u0073\u0020\u0062\u0061\u0063\u006b\u0020\u0075\u0070\u0020"
    "\u0074\u0068\u0061\u0074\u0020\u0054\u0075\u0072\u0074\u006c\u0065\u002e\u002e"
    "\u002e\n\n\u0049\u0074\u0020\u0073\u0065\u0065\u006d\u0073\u0020\u0079\u006f"
    "\u0075\u0027\u0076\u0065\u0020\u0066\u006f\u0072\u006b\u0065\u0064\u0020\u0074"
    "\u0068\u0065\u0020\u0054\u0075\u0072\u0074\u006c\u0065\u0043\u006f\u0069\u006e"
    "\u0020\u0072\u0065\u0070\u006f\u0073\u0069\u0074\u006f\u0072\u0079\u0020\u0061"
    "\u006e\u0064\u0020\u0075\u0073\u0065\u0064\u0020\u0061\u0020\u0073\u0069\u006d"
    "\u0070\u006c\u0065\u0020\u0072\u0065\u0070\u006c\u0061\u0063\u0065\u0020\u0061"
    "\u006c\u006c\u0020\u0074\u006f\u0020\u006d\u0061\u006b\u0065\u0020\u0074\u0068"
    "\u0065\u0020\u0070\u0072\u006f\u006a\u0065\u0063\u0074\u0020\u0079\u006f\u0075"
    "\u0072\u0020\u006f\u0077\u006e\u002e\n\n\u0059\u006f\u0075\u0027\u0076\u0065"
    "\u0020\u006c\u0069\u006b\u0065\u006c\u0079\u0020\u0061\u006c\u0074\u0065\u0072"
    "\u0065\u0064\u0020\u006c\u0069\u0063\u0065\u006e\u0073\u0065\u0020\u0068\u0065"
    "\u0061\u0064\u0065\u0072\u0073\u0020\u0061\u0073\u0020\u0061\u0020\u0072\u0065"
    "\u0073\u0075\u006c\u0074\u0020\u0077\u0068\u0069\u0063\u0068\u0020\u0069\u0073"
    "\u0020\u0069\u006e\u0020\u0076\u0069\u006f\u006c\u0061\u0074\u0069\u006f\u006e"
    "\u0020\u006f\u0066\u0020\u0074\u0068\u0065\u0020\u006f\u0070\u0065\u006e\u0020"
    "\u0073\u006f\u0075\u0072\u0063\u0065\u0020\u006c\u0069\u0063\u0065\u006e\u0073"
    "\u0065\u002e\n\n\u0050\u0065\u0072\u0068\u0061\u0070\u0073\u0020\u0079\u006f"
    "\u0075\u0027\u0064\u0020\u006c\u0069\u006b\u0065\u0020\u0074\u006f\u0020\u0074"
    "\u0061\u006b\u0065\u0020\u0061\u0020\u006c\u006f\u006f\u006b\u0020\u0061\u0074"
    "\u0020\u0068\u0074\u0074\u0070\u0073\u003a\u002f\u002f\u0074\u0075\u0072\u0074"
    "\u006c\u0065\u0063\u006f\u0069\u006e\u002e\u0067\u0069\u0074\u0068\u0075\u0062"
    "\u002e\u0069\u006f\u002f\u0066\u006f\u0072\u006b\u002f\u0020\u0074\u006f\u0020"
    "\u0066\u0069\u006e\u0064\u0020\u006f\u0075\u0074\u0020\u0068\u006f\u0077\u0020"
    "\u0074\u006f\u0020\u0066\u006f\u0072\u006b\u0020\u0074\u0068\u0065\u0020\u0070"
    "\u0072\u006f\u006a\u0065\u0063\u0074\u0020\u0063\u006f\u0072\u0072\u0065\u0063"
    "\u0074\u006c\u0079\u003f\n\n\u0049\u0066\u0020\u0079\u006f\u0075\u0020\u006e"
    "\u0065\u0065\u0064\u0020\u0068\u0065\u006c\u0070\u0020\u006d\u0061\u006b\u0069"
    "\u006e\u0067\u0020\u0073\u0075\u0072\u0065\u0020\u0079\u006f\u0075\u0027\u0072"
    "\u0065\u0020\u0064\u006f\u0069\u006e\u0067\u0020\u0074\u0068\u0069\u006e\u0067"
    "\u0073\u0020\u0072\u0069\u0067\u0068\u0074\u002c\u0020\u0073\u0077\u0069\u006e"
    "\u0067\u0020\u006f\u006e\u0020\u0062\u0079\u0020\u0068\u0074\u0074\u0070\u003a"
    "\u002f\u002f\u0063\u0068\u0061\u0074\u002e\u0074\u0075\u0072\u0074\u006c\u0065"
    "\u0063\u006f\u0069\u006e\u002e\u006c\u006f\u006c\u0020\u0061\u006e\u0064\u0020"
    "\u0077\u0065\u0027\u006c\u006c\u0020\u0062\u0065\u0020\u0068\u0061\u0070\u0070"
    "\u0079\u0020\u0074\u006f\u0020\u0068\u0065\u006c\u0070\u002e\n\n\n");

