# Utility

This library provides a objects and functions to do spesifict purpose.

## Actions

This repo was made with [nob.h](https://github.com/tsoding/nob.h) with some command and flags as argument

To Usage: ./nob \<command\> \<flags\>
<table>
  <tr><th>Command</th><th>Description</th></tr>
  <tr>
    <td>h[elp]</td>
    <td>show this message.</td>
  </tr>
  <tr>
    <td>t[ests]</td>
    <td>make and run all test.</td>
  </tr>
  <tr>
    <td>c[lean]</td>
    <td>clean generated binary files/folders.</td>
  </tr>
  <tr>
    <td>s[tatus]</td>
    <td>show current device stat.</td>
  </tr>
  <tr>
    <td>rand_test</td>
    <td>run random test.</td>
  </tr>
  <tr>
    <td>matrix_test</td>
    <td>run matrix test.</td>
  </tr>
  <tr>
    <td>complex_test</td>
    <td>run complex test.</td>
  </tr>
  <tr>
    <td>bigInteger_test</td>
    <td>run bigInteger test.</td>
  </tr>
  <tr>
    <td>hash_test</td>
    <td>run hash test.</td>
  </tr>
  <tr>
    <td>sort_test</td>
    <td>run sort test.</td>
  </tr>
  <tr><th>Flags</th><th>Description</th></tr>
  <tr>
    <td>-b,--build</td>
    <td>Keep build exec, even already updated.</td>
  </tr>
  <tr>
    <td>-d,--debug</td>
    <td>Run exec in debug mode. always removed after.</td>
  </tr>
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
