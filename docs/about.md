---
layout: page
breadcrumb_title: About
weight: -90
navigation:
  visible: true
  title: About
---

## About
Guild Wars 2 Add-on Loader acts as a master add-on in which you can enable or disable other individual add-ons without having to fiddle around with files in the Guild Wars 2 folder.
It will check for additional add-ons and load them regardless of the filenames, in contrast to what traditional methods do.

### How it works
This add-on is intended to be used as your main add-on.
Additional add-ons have to be placed in a certain folder that the add-on loader checks when Guild Wars 2 is starting up.
These add-ons will be made available in the user interface of the add-on loader.
You choose which add-ons are actually enabled, and in which order they are executed.

The add-on loader forwards all the necessary functions to these add-ons in the order that you choose.
These add-ons will do whatever they need to do in those functions, and return to the add-on loader once they are done.
This is very much like traditional chainloading, however in this case the add-on loader takes care of it all and those add-ons don't have to implement additional support for chainloading themselves anymore.

Of course, add-ons can still have their own additional plugins built into them.
If that's the case, the add-on loader has no influence on that and the state of those plugins are managed by that add-on instead.

### Compatibility
There are two types of add-ons: native and legacy.

Native add-ons are fully compatible.
They are developed against our API that enables special features for these add-ons.

Legacy add-ons use traditional D3D9 hooking in order to do their work.
Due to the nature of how this system works, compatibility for these add-ons is highly experimental.
Using these add-ons may crash your game.
Support for these add-ons will be given a low priority.
It's your best bet to ask the add-on developer to make it compatible by converting it to a native add-on.

If you're an add-on developer and want to make your add-on compatible, it's a good start to take a look at the examples in the repository.

### ArenaNet
Of course, making and releasing add-ons for Guild Wars 2 will always include the usual disclaimer.
This add-on is unofficial and is not supported nor endorsed by ArenaNet.
Using this software is at your own risk; there's no warranty.
If you do not agree with these terms, please do not use this software.

ArenaNet has not yet responded on their stance regarding this add-on.
This means that, while this add-on doesn't interact with nor does it provide any advantages in the gameplay in any way, this add-on is still in a gray area.
You decide whether or not it's worth using this add-on.
Once a response is received, this will be updated to reflect that response.

### Contributing
People helping developing this add-on is always appreciated.
Please use your common sense when making an issue or a pull request on GitHub.
Keep in mind that new feature requests should not be out-of-scope of the project.
Your request can always be denied on the author's discretion.

### Contact
You can contact me on [GitHub][github], [Twitter][twitter], [Reddit][reddit] or Discord (soon).
Keep in mind that the Discord server is a generic Discord server for developers that build against the Guild Wars 2 API, and not specifically for this add-on only.
Make sure you use the correct channel.

Unless it's urgent, please do not contact me in-game.
When I'm online in Guild Wars 2, I want to play the game instead.

[github]: {{ site.github.repository_url }}
[twitter]: https://twitter.com/{{ site.twitter.username }}
[reddit]: https://reddit.com/u/{{ site.reddit.username }}
