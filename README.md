# Cocktail Recipe for PG+
A little command that gives you cocktail recipes.

## Usage

```
$ cocktail old fashioned

==================== Cocktail recipe for: Old Fashioned ===================

Old Fashioned: 
A pre-dinner drink with a boozy taste.

Ingredients:

- Rye Whiskey: 6 cl
- Simple syrup: 1 cl
- Angostura bitters: 2 dashes

Preparation: Stirred.
  
===========================================================================
````

## Live Example
This code is running on the [UberWorld](http://uberworld.org) talker.

## Requirements

A functioning [Playground Plus](https://github.com/talkers/pgplus) talk server.

## Installation

1. Add `src/cocktail.c` provided in this repo to your talker's `src` directory.

1. Update `src/include/clist.h` by inserting the following code at ~[line 165](https://github.com/talkers/pgplus/blob/master/src/include/clist.h#L165):

    ```
    extern command_func cocktail;
    ```

1. Update `src/include/clist.h` by inserting the following code at ~[line 610](https://github.com/talkers/pgplus/blob/master/src/include/clist.h#L610):

    ```
    {"cocktail", cocktail, 0, 0, 1, 0, MISCc},
    ```

1. Update `doc/help` by inserting the following code at the end:

    ```
     :cocktail

     ^ZCommand  :^N cocktail <drink>

     ^ZDetails  :^N Request a recipe for the tasty beverage of your choice.

     ^ZNotes    :^N * Try `cocktail list` if you want to browse the recipe list.
    ```

1. Add the file `files/cocktails` provided in this repository to your talker's `files` directory.

1. Recompile using the standard PG+ compilation process.

## You might also like
* [PG+ Short Link Generator](https://github.com/jmodjeska/pgplus-shortlink)
* [PG+ Test Harness](https://github.com/jmodjeska/pgplus-test)
