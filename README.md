# SLEther

SLIP for Ethernet. We're a whole layer deeper now.

## Why?

I wanted to connect a device to a network through a serial line, and I wasn't
satisfied with existing layer 3 options because I wanted it to receive router
advertisements. Maybe PPP can be hacked to make this work, but it seemed
easier just to write a TAP-based driver.

## What's the protocol?

Ethernet frame encapsulated with SLIP, as described in RFC 1055.

### Doesn't SLIP only apply to IP packets?

It's basically just byte stuffing with a fancy name.

## Supported platforms

This was written on and for NetBSD. I aim to make it compatible with Linux soon.

## Licensing

No licensing at this time. As this is an unfinished product with serious bugs and
missing features, please do not redistribute.

<hr>

Copyright &copy; Robbie 2021
