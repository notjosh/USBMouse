USB Mouse
=========

So you're at your mother's house, poking around, and you find a USB mouse on her desk. It's not any ordinary USB mouse, it's a literal mouse (albeit plastic) on an exercise bike.

![USB mouse](http://hi.notjo.sh/0Z2H0n0j092L/IMG_6325.JPG)

You find the driver CD, but realise it only supports Windows while all you have is macOS. You realise she has a Windows machine, but upon installation you find out the last supported version is Windows XP, and, well, we're a little past that at this point.

The only thing left to do is...reverse the communications protocol, and implement the driver yourself.

Bit of Background
=================

The Windows program is focused on typing - the faster you type, the faster the mouse pedals. The LCD is used to show how many words you've typed today.

In short, that leaves the communication protocol as:

 - Set LCD value
 - Increment LCD value
 - Set pedal speed

The company behind it is 50fiftyconcepts (today known as [50-fifty gifts](http://www.50fifty-gifts.com/)) if you want to track down your very own USB mouse!

The chip used is a Megawin MA-101. It's exciting, because there's a [data sheet](www.megawin.com.tw/zh-tw/support/downloadDocument/106/186) still kicking around. However, the closest it gets to any kind of software API documentation is reference to (occasionally misspelled) `BridgeMini.dll`. So, it's pretty useless for us here.

Some other multimedia assets:

 - [Feels](http://hi.notjo.sh/2p0f1Q2J1U2K) (image)
 - [Shell](http://hi.notjo.sh/3i020s262m3C) (image)
 - [Pedalling action](http://hi.notjo.sh/2s442Y402b2E) (video (vertical, sorry))

Protocol
========

The commands are sent to look like one of:
 
 - Handshake 1
 - Handshake 2
 - Poke

(I don't exactly know why, but two handshake commands are necessary.)

All are host-to-device requests.

The poke commands are:

 - Set LCD value
 - Increment LCD value
 - Set pedal speed

Command: Handshake 1
--------------------

bRequest: `0xc`<br>
wValue: `0x5003`<br>
wIndex: `0xffff`<br>
wLength: `0x1`<br>
data: `0x8`

Command: Handshake 2
--------------------

bRequest: `0xc`<br>
wValue: `0x5003`<br>
wIndex: `0xfff0`<br>
wLength: `0x1`<br>
data: `0x10`

Command: Poke
-------------

bRequest: `0xc`<br>
wValue: `0x5001`<br>
wIndex: `0x0`<br>
wLength: `0x1`<br>
data: (as described below in commands)

I'll use `poke()` to signify that commands will be sent using the above setup packet.

Poke Command: Set LCD value
---------------------------

```
poke(0xA)        // begin stream
poke(character1)
poke(character2)
poke(character3)
poke(character4)
poke(character5)
poke(0x39)       // end stream
poke(0x5)        // refresh LCD
```

Where `characterX` is (from left to right) one of the following:

`0x0`: 0<br>
`0x1`: 1<br>
`0x2`: 2<br>
`0x3`: 3<br>
`0x4`: 4<br>
`0x5`: 5<br>
`0x6`: 6<br>
`0x7`: 7<br>
`0x8`: 8<br>
`0x9`: 9<br>
`0xA`: (space)<br>
`0xB`: F<br>
`0xC`: L<br>
`0xD`: b<br>
`0xE`: A<br>
`0xF`: E

Poke Command: Increment LCD value
---------------------------------

```
poke(0x66)
```

It's smart enough to carry decimals as base-10, not base-10.

Poke Command: Set pedal speed
-----------------------------

```
poke(0x80 | speed)
```

Where `speed` is a value between `0x0` and `0x7f`. In practice, any values below `0x10` are too slow for the motor, and friction will keep it from turning. Values above `0x40` seem to be the same in terms of RPM. YMMV.

TODO
====

I've got ideas for more library functions, like

 - marquee text
 - justify text

I'd also like to use [SMCKit](https://github.com/beltex/SMCKit) and [SystemKit](https://github.com/beltex/SystemKit) to wrap up CPU usage and fan speed. But, stupidly, I updated to 10.12 on day one that DP1 dropped, so predictably things are a little broken.

Pull requests are, of course, very welcome.
