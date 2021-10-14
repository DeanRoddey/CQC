#  The CQC Automation System

**What It Is**

CQC is a commercial quality, software based automation system, suitable for residential or commercial applications. If you aren't familiar with automation systems, this read me isn't the place to learn about that fairly large subject. There's a lot of information out there about them. You can also read through the broad overview in the CQC documentation (which will be available locally if you install it or build it, but it's also available online until then) at:

https://www.charmedquark.com/Web2/CQCDocs/CQCDocs.html?topic=/&page=/Overview

See the 'Automation Overview' topic on the left. You can also get a lot of information from the CQC Youtube channel as well. This will be CQC specific content but a lot of it will be generally applicable.

https://www.youtube.com/user/CharmedQuarkSystems/playlists

But, just to give a very simple list of the types of functionality CQC provides:

* Obviously control over devices is at the core of automation, to get information from them (and make it available internally in a consistent form) and to send out commands to devices.
* Powerful touch screen UI design which lets you visually expose device functionality to your users in a highly customized way.
* React to changes in device state and do things (turn on the lights when a motion sensor triggers, etc...)
* Invoke operations on a scheduled basis (turn on outside lights just before sunset, arm security system after a particular time, remind you to take out the trash on tuesday, etc...)
* Control devices via various means, e.g. phones, computers, remote controls, voice, wall mounted panels, motion, etc...)
* Have the system speak to you with important information.
* Manage media and media systems.
* Create your own custom clients that consume device data or control devices in some useful way.
* To combine multiple device states into single composite values so that they are much more convenient to react to or deal with.
* Send e-mails or texts when important changes in devices occur
* Create custom logic, which can be invoked in the various ways mentioned above


**Platform Support**

CQC is Windows based, but it supports non-Windows touch screen clients, which is where cross platform support is important. You will though need a Windows machine to install it on and for management. CQC has a Windows based flagship touch screen client, and it has a browser based client for phones and non-Windows tablets. The same interfaces are available on both, so you don't need separate UI designers.

CQC is a network distributed system, and you can install various parts of it on multiple machines in your local network, and manage them all from any CQC enabled Windows machine.

The back end could be available on Linux, but that requires that the work to get the underlying CIDLib system available on Linux to be completed. It used to be available, but that wasn't used for a long time and so that support sort of atrophied and needs to be redone. See the next section.


**CIDLib Based**

CQC is based on our very powerful CIDLib C++ development system, and it demonstrates the power and cleanliness of CIDLib as a development platform. There are no standard C++ or third party interfaces involved, it's all in terms of CIDLib APIs.

* Well, there are a few small places that haven't been abstracted down to CIDLib yet, but it's 99% in terms of CIDLib APIs.

This means that, if you want to build CQC, you will also need to load the CIDLib repository and build that first. They are separate repositories but designed to work together very easily. They both use the CIDLib build tool which fully understands all of the tools and requirements of CIDLib based development.

https://github.com/DeanRoddey/CIDLib


**Build Process**

The build process is now fairly well documented in the help. You will have the help available locally, but have to install and build CIDLib/CQC first so you can build the help. So initially you'll want to look at the online version of the help on the web site. Use the Advanced Topics link on the left. Here is a direct link to that section:

https://www.charmedquark.com/Web2/CQCDocs/CQCDocs.html?topic=/AdvancedTopics/CQCDevelopment&page=/Overview


**Git Issues**

Since we have two separate Git repositories, you can't use Git from within Visual Studio Code since it can only understand one Git repository, but you definitely want it to see both code bases for all the obvious practical purposes. So I set my VSC project directory in the shared directory above the two code bases (the CQSL directory in the example above.)

So all Git operations are done outside of the IDE, at least if you use VSC. Visual Studio Code is available on Windows and Linux and provides an identical experience on both, so I'd definitely recommend looking into it. Whatever anyone uses we need to make sure Git ignores any housekeeping files they generate.


**History**

CQC builds on the history of CIDLib, so see the Readme.md in the CIDLib repository. CQC was initially started just to have something to do with the massive amount of general purpose code provided by CIDLib. It was also started as a commercial undertaking. Though, being business impaired, it was not because of any actual proof that this would be a good business venture. I just was interested in automation.

In retrospect, being business impaired isn't probably a good thing if you are going to go into business. We were never really albe to grow a viable market for CQC. It was too powerful for the average user, and the professional installers are extremely sensitive to the long term viability of the products they depend on, and the automation system is at the core of the business. Then of course a lot of large companies got into the lower end of automation, and the traditional high end, proprietary hardware based systems started migrating downwards as well.

After a couple decades of ever declining prospects, we open sourced the product. Hopefully it will attract developers who can help to leverage the massive power inherent in this system. It could be much more than just an automation system.


**License**

Both CQC and CIDLib are available under the MIT open source license.

https://opensource.org/licenses/MIT

