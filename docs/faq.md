---
layout: page
breadcrumb_title: Frequently Asked Questions
weight: -70
navigation:
  visible: true
  title: FAQ
---

## Frequently Asked Questions
Where do I download / How do I install the add-on loader?
: Go to the [installation page][install].

Will I get banned from using this?
: *Probably not.*
  However, ArenaNet has not stated whether or not they are fine with this add-on.
  So you're entirely on your own if your account does get banned.
  Do check out the [about page][about] under the section *ArenaNet*.
  Also, while it's unlikely to get banned from using this add-on, this doesn't mean that you can automatically use any other add-on without risking getting banned.
  Every add-on has its own disclaimer.
  Use your own judgment to decide whether or not using an add-on is allowed within the Guild Wars 2 Terms of Service.

How do I open the add-ons list in-game?
: Use the keybind to open the window. It's Alt + Shift + F11 by default, and can be changed in the settings.

The keybind is not opening the add-ons list.
: Are you sure the add-on loader is installed properly?
  Do you have the correct version installed?
  Or did you change the keybind to something else perhaps?
  You can always check if the add-on loader is loaded by going to the *addons/loader* folder inside your Guild Wars 2 installation folder, and opening the *loader.log* file.
  If everything seems to be in order, you can also delete the *loader.ini* file.
  This will reset all your settings to the default settings, and therefore also the keybind.

I want to report a bug or a crash related to the add-on loader.
: Great!
  Submit it as [an issue on the repository][issues] with the log if necessary (which can be found as *addons/loader/loader.log* inside the Guild Wars 2 installation folder).
  Please do check if it has been reported already.

What are the diagnostics?
: Those are mostly statistics of various things.
  For example, it will show you how an add-on is performing, how many CPU time the game is consuming, etc.
  If you don't require the diagnostic features, please turn them off, because leaving the diagnostics enabled might affect game performance.

Why is &lt;insert some add-on name&gt; not working?
: Are you sure it's compatible and that you placed it in the correct folder?
  If it's a legacy add-on and it crashes your game, your best bet is to ask the developer of the add-on to make it compatible.
  Support for legacy add-ons will be given a low priority here.  
  If it's not showing up in the list, it might not be compatible at all.
  You can enable the listing for incompatible add-ons in the settings, however this won't allow you to enable those add-ons.
  Please contact the developer of the add-on to make it compatible.

How do I make my add-on compatible?
: Go to the [repository][repository] and check out the readme and the provided examples.

[install]: {{ site.baseurl }}{% link install.md %}
[about]: {{ site.baseurl }}{% link about.md %}
[repository]: {{ site.github.repository_url }}
[issues]: {{ site.github.issues_url }}
