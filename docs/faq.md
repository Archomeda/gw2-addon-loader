---
layout: page
breadcrumb_title: Frequently Asked Questions
weight: -70
navigation:
  visible: true
  title: FAQ
---

## Frequently Asked Questions
Where do I download / How do I install the addon loader?
: Go to the [installation page][install].

Will I get banned from using this?
: *Probably not.*
  However, ArenaNet has not stated whether or not they are fine with this addon.
  So you're entirely on your own if your account does get banned.
  Do check out the [about page][about] under the section *ArenaNet*.
  Also, while it's unlikely to get banned from using this addon, this doesn't mean that you can automatically use any other addon without risking getting banned.
  Every addon has its own disclaimer.
  Use your own judgment to decide whether or not using an addon is allowed within the Guild Wars 2 Terms of Service.

How do I open the addons list in-game?
: Use the keybind to open the window. It's Alt+Shift+F11 by default, and can be changed in the settings.

The keybind is not opening the addons list.
: Are you sure the addon loader is installed properly?
  Do you have the correct version installed?
  Or did you change the keybind to something else perhaps?
  You can always check if the addon loader is loaded by going to the *addons/loader* folder inside your Guild Wars 2 installation folder, and opening the *loader.log* file.
  If everything seems to be in order, you can also delete the *loader.ini* file.
  This will reset all your settings to the default settings, and therefore also the keybind.

I want to report a bug or a crash related to the addon loader.
: Great!
  Please submit it as [an issue on the repository][issues] with the log if necessary (which can be found as *addons/loader/loader.log* inside the Guild Wars 2 installation folder).
  Do check if it has been reported already.

What are the debug features?
: Those are mostly meant for debugging addons.
  It will show you how an addon is performing.
  Right now it doesn't contain much else than that, but it might be expanded upon in the future.
  If you don't require the debug features, please disable them.

Why is &lt;insert some mod name&gt; not working?
: Are you sure it's compatible and that you placed it in the correct folder?
  If it's not compatible, the addon loader won't show you the addon in the list by default.
  You can enable the listing for incompatible addons in the settings.
  However, this won't allow you to enable those addons.
  If the addon is incompatible, please contact the author of that addon and ask them to make it compatible.

How do I make my addon compatible?
: Go to the [repository][repository] and check out the readme and the provided examples.

[install]: {{ site.baseurl }}{% link install.md %}
[about]: {{ site.baseurl }}{% link about.md %}
[repository]: {{ site.github.repository_url }}
[issues]: {{ site.github.issues_url }}
