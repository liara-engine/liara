---
title: Core principles
description: The six principles every architectural decision is evaluated against, and what each one is actually protecting.
sidebar:
  order: 1
---

These are the lens every architectural decision is held up to. When a tradeoff appears, the principle wins by default, and an override is documented rather than made quietly.

## KISS, as a cognitive discipline

The simplest design meeting the stated requirements wins. That is not an aesthetic position: it is a survival mechanism for a solo developer who cannot afford to drown in accidental complexity. Wherever a decision can be made smaller, less coupled or more local, it should be.

## Modular by construction, not by convention

A module is not modular because it lives in its own folder. It is modular when removing it, replacing it or recompiling it on its own is mechanically possible.

Every claim of modularity gets one question: could somebody reimplement this in another language and link it in instead? If the answer is no, it is not a module, whatever the directory structure says.

## Interfaces before implementation

The boundary between modules matters more than the inside of them. Interfaces are designed first, frozen carefully and changed reluctantly. Implementations may iterate as freely as they like.

## Performance is a design choice, not an optimization pass

Memory layout, allocation patterns, cache behavior and threading model are decided at design time. The engine does not write naive code intending to profile later. It writes code that has already thought about performance, and profiles to find out whether the thinking was right.

## Visible progress at every step

No version is allowed to be internal refactoring with nothing to show. Every release produces something demonstrable: a binary that runs, a test that passes, a piece of documentation that explains something new.

This one is about motivation rather than engineering, and it is load-bearing for exactly that reason. A milestone that produces nothing visible is a milestone that does not get finished.

## The future reader is the author

The author six months from now is a different person, with none of the context that currently lives only in my head, and the project has to be navigable by that person.

Two things follow. Decisions are written down when they are made rather than reconstructed later, which is what the decision records are for. And consistency across the project matters more than local cleverness, because the future reader recognizes a pattern faster than they reconstruct an argument.
