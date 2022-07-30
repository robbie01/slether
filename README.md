# SLEther

SLIP for Ethernet. We're a whole layer deeper now.

## Why?

I wanted to connect a device to a network through a serial line, and I wasn't
satisfied with existing layer 3 options because I wanted it to receive router
advertisements. ~~Maybe PPP can be hacked to make this work, but it seemed
easier just to write a TAP-based driver.~~ Just use PPP with EtherIP on NetBSD,
or PPP with VXLAN on FreeBSD, OpenBSD, or Linux. I haven't tried it myself, but
it should be a much more robust solution than hacking together a userspace
driver.

## What's the protocol?

Ethernet frame encapsulated with SLIP, as described in RFC 1055.

### Doesn't SLIP only apply to IP packets?

It's basically just byte stuffing with a fancy name.

## Supported platforms

This was written on and for NetBSD, and almost certainly only supports NetBSD.

## Licensing

This software is offered under the Fortnite&reg; End User License Agreement,
the terms of which are present in the `LICENSE` file of this repository.

<hr>

Copyright &copy; Robbie 2021-2022
