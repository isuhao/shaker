Shaker
======

PPAPI/NaCl utilities

Install
-------

After downloading/cloning the Shaker you need to build the Windows library. For that, you need to show Shaker, where your Pepper is:

```
> python nacl_sdk.py <your-nacl-sdk-dir>\pepper_<version>
```

This will finalize the Shaker projects and solution. After that you should build `shaker.sln` in Debug and in Release.

Using
-----

There is a script called `generate.py` which will generate a directory with an empty Pepper plugin with files for the Visual Studio. `generate.py --help` will tell you, what options are available.

Example:

```
> python <shaker-dir>\generate.py --name FanasticPlugin \
  --mime-type application/x-ppapi-fantastic-plugin --git
> dir fanastic_plugin
 Volume in drive X is Name
 Volume Serial Number is XXXX-XXXX

 Directory of fanastic_plugin

2014-08-21  15:25    <DIR>          .
2014-08-21  15:25    <DIR>          ..
2014-08-21  15:25    <DIR>          .git
2014-08-21  15:25                53 .gitignore
2014-08-21  15:25               623 fanastic_plugin.html
2014-08-21  15:25               801 fanastic_plugin.props
2014-08-21  15:25               960 fanastic_plugin.rc
2014-08-21  15:25             2 277 fanastic_plugin.vcxproj
2014-08-21  15:25             1 325 fanastic_plugin.vcxproj.filters
2014-08-21  15:25               942 fanastic_plugin.vcxproj.user
2014-08-21  15:25               280 Instance.cpp
2014-08-21  15:25               259 Instance.hpp
2014-08-21  15:25               364 Module.cpp
2014-08-21  15:25               499 shaker_home.props
              11 File(s)          8 383 bytes
               3 Dir(s)                 bytes free

```

The default settings for the debugging will use your primary Chrome installation. You can use `$chrome_sxs` in Debugging &gt; Command&nbsp;Arguments, if you want `pp-launcher` to look for Chrome Canary. If you want to use some other binary, make sure first argument to `pp-launcher` points to the full path of `chrome.exe`.

HTML markup
-----------

Pepper plugins need the `type` of their own. Use the same string you used in `--mime-type`:
```
<embed width="320" height="240" type="application/x-ppapi-fantastic-plugin" />
```

If Chrome does not want to load your DLL, but `chrome://plugins` still show your plugin, make sure the DLL is **not** on the mapped network drive. If you cannot build the DLL localy, copy it to a local drive and run `pp-launcher` from there (remeber to point it to your Chrome binary).
