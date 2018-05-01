---
layout: page
breadcrumb_title: Installation
weight: -80
navigation:
  visible: true
  title: Installation
---

{% include install.html %}

## Installation
This page will help you to install the addon loader.

<div id="releases" {% unless site.github.latest_release and site.github.latest_release.assets %}class="hidden"{% endunless %}>
  First you'll have to download the <a href="{{ site.github.latest_release.html_url}}" id="latest-release-link">latest release (<span id="latest-release-version">{{ site.github.latest_release.tag_name }}</span>)</a>:

  <ul id="latest-release-assets">
    {% for asset in site.github.latest_release.assets %}
      <li><a href="{{ asset.browser_download_url }}">{{ asset.name }}</a></li>
    {% endfor %}
  </ul>
</div>
<div id="releases-generic" {% if site.github.latest_release and site.github.latest_release.assets %}class="hidden"{% endif %}>
  First you'll have to download the latest release from the <a href="{{ site.github.releases_url }}">GitHub repository</a>.
</div>

You'll have to choose between the 32-bit (x86) or the 64-bit (x64) version for Windows, depending on your Guild Wars 2 installation (OSX is not supported).
If you don't know which version you need, it's most likely 64-bit.

Afterwards, you extract everything from the zip file into the following folder inside your Guild Wars 2 installation folder: *bin* (32-bit) or *bin64* (64-bit).

And that's it.
However, the addon loader on its own doesn't do much.
You'll need additional addons.

### Installing additional addons
Installing additional compatible addons is fairly easy.
Place the addon in the following folder inside your Guild Wars 2 installation folder: *bin/addons* (32-bit) or *bin64/addons* (64-bit).
Create the folder if it doesn't exist already.
It will show up in the addon list the next time Guild Wars 2 is restarted (default keybind is Alt + Shift + F11).
Don't forget to activate it!

**Note:** If the addon is a legacy addon, you'll be greeted with a warning.
Please make sure you understand the consequences when using legacy addons.
Any *additional* file that's included with a legacy addon needs to be placed in the original location as if you were not using the addon loader.
Follow the instructions provided by the addon.
