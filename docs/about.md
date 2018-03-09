---
layout: page
breadcrumb_title: About
weight: -90
navigation:
  visible: true
  title: About
---

## About
Guild Wars 2 Addon Loader acts as a master addon in which you can enable or disable other individual addons without having to fiddle around with files in the Guild Wars 2 folder.
It will check for additional addons and load them regardless of the filenames, in contrast to what traditional methods do.

### How it works
This addon is intended to be used as your main addon.
Additional compatible addons have to be placed in a certain folder that the addon loader checks when Guild Wars 2 is starting up.
These addons will be made available in the user interface of the addon loader.
You choose which addons are actually enabled, and in which order they are executed.

The addon loader forwards all the necessary functions to these addons in the order that you choose.
These addons will do whatever they need to do in those functions, and return to the addon loader once they are done.
In contrast, this is very much like traditional chainloading, however in this case, the addon loader takes care of it all and those addons don't have to implement support for chainloading themselves anymore.

Of course, addons can still have their own additional plugins built into them.
If that's the case, the addon loader has no influence on that and the state of those plugins are managed by that addon instead.

### Compatibility
Due to the nature of how this system works, addons that use the traditional D3D9 hooking are not compatible.
Addon developers have to make their addon specifically compatible with the addon loader.
It's not too difficult to make addons compatible, depending on the features that the addon needs.
There's an example in the repository that shows how to develop addons against the addon loader.

### ArenaNet
Of course, making and releasing addons for Guild Wars 2 will always include the usual disclaimer.
This addon is unofficial and is not supported nor endorsed by ArenaNet.
Using this software is at your own risk; there's no warranty.
If you do not agree with these terms, please do not use this software.

ArenaNet has not yet responded on their stance regarding this addon.
This means that, while this addon doesn't interact with nor does it provide any advantages in the gameplay in any way, this addon is still in a gray area.
You decide whether or not it's worth using this addon.
Once a response is received, this will be updated to reflect that response.

### Contributing
People helping developing this addon is always appreciated.
Please use your common sense when making an issue or a pull request on GitHub.
Keep in mind that new feature requests should not be out-of-scope of the project.
Your request can always be denied on the author's discretion.

### Contact
You can contact me on [GitHub][github], [Twitter][twitter], [Reddit][reddit] or Discord (soon).
Keep in mind that the Discord server is a generic Discord server for developers that build against the Guild Wars 2 API, and not specifically for this addon only.
Make sure you use the correct channel.

Unless it's urgent, please do not contact me in-game.
When I'm online in Guild Wars 2, I want to play the game instead.

[github]: {{ site.github.repository_url }}
[twitter]: https://twitter.com/{{ site.twitter.username }}
[reddit]: https://reddit.com/u/{{ site.reddit.username }}
