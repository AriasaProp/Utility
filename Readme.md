# Utility

This library provides a objects and functions to do spesifict purpose.

## Actions

This repo was made with [nob.h](https://github.com/tsoding/nob.h) with some command and flags as argument

To Usage: run/build <command> [group|kind] [flags]
<table>
  <tr><th>Command</th><th>Description</th></tr>
  <tr><td>h[elp]</td><td>show this message.</td></tr>
  <tr><td>c[lean]</td><td>clean generated binary [group] files/folders or all.</td></tr>
  <tr><td>s[tatus]</td><td>show current device [group] or all.</td></tr>
  <tr><td>test</td><td>run exists test or all.</td></tr>
  <tr><td>benchmark</td><td>run exists benchmark or all.</td></tr>
  <tr><th>Group</th><th>Description</th></tr>
  <tr><td>test</td><td>test [group]</td></tr>
  <tr><td>benchmark</td><td>benchmark [group]</td></tr>
  <tr><th>Flags</th><th>Description</th></tr>
  <tr><td>-b,--build</td><td>Keep build exec, even already updated.</td></tr>
  <tr><td>-d,--debug</td><td>Run exec in debug mode. always removed after.</td></tr>
</table>


## Parts

* **Math:** This module provides a set of mathematical objects and operations.
    * `bigInteger`:  Implements integer arithmetic.
    * `matrix`:  Provides matrix manipulation functionality.
    * `complex`:  Implements complex number arithmetic.

* **Codec:** This module handles encoding and decoding of data into various formats.
    * `image`:  transform bitmap input into image format.
    * `json`:  read and write json data.

* **Algorithm:** This module store algorithm.
    * `sort`: store algorithm for sorting object.
    * `hash`: store algorithm for generate secure fixed random hasher.
