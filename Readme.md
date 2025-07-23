<p align="center"><img src="https://github.com/user-attachments/assets/a1b5abfc-f1e5-4615-bd7c-358b389afbd8" alt="Logo" width="300"></p>

<h1 align="center"> Eurekiel Feature Test</h1>
<h4 align="center">Testbed for the Eurekiel engine Testing the feasibility and stability of features</h4>
<p align="center">
<a href="https://www.codefactor.io/repository/github/caishangqi/Eurekiel"><img src="https://www.codefactor.io/repository/github/caishangqi/EnigmaVoxel/badge" alt="CodeFactor" /></a>
<img alt="Lines of code" src="https://img.shields.io/badge/Render API-DirectX12 | DirectX11 | OpenGL-242629">
<img alt="Render Backend" src="https://img.shields.io/badge/C++-17-cherry">
<img alt="GitHub branch checks state" src="https://img.shields.io/github/checks-status/Caishangqi/Eurekiel/master?label=build">
<img alt="GitHub code size in bytes" src="https://img.shields.io/github/languages/code-size/Caishangqi/Eurekiel">
</p>

## Overview

Test the Eurekiel engine's feature testing project, which categorizes the modules to be added and tests them in order, while the tested content will be kept in the project and submitted to the main branch of Eurekiel.
## Feature

> Description of the feature here. This could be a feature that adds new functionality, improves existing features, or enhances the overall performance of the engine.

## Planned Feature

- Support Blindless rendering options, enable by engine.configuration.
- Support for multiple render backends (DirectX12, DirectX11, OpenGL).


### Rendering Pipeline

TBD

### Voxel Module

TBD

### Resource Module

TBD

## Modules

| **Name**               |                               **Description**                               |     **State**     |
|------------------------|:---------------------------------------------------------------------------:|:-----------------:|
| `enigma::core`         |             The Core structure and classes that used in engine              |      stable       |
| `enigma::network`      |       The Network subsystem that handles client and server behaviour        |    unavailable    |
| `enigma::audio`        | Audio subsystem that implement FMOD API wrapper functions and encapsulation |      stable       |
| `enigma::input`        |                 The input subsystem that use the XInput API                 |      stable       |
| `enigma::math`         |                 The Engine math datastructures and geometry                 | unstable/refactor |
| `enigma::render::dx11` |     The DirectX 11 Renderer API that implements the rendering pipeline      |      stable       |       beta        |
| `enigma::render::dx12` |     The DirectX 12 Renderer API that implements the rendering pipeline      |       beta        |       beta        |
| `enigma::resource`     |              The Namespace resource register and cache system               |    unavailable    |
| `enigma::window`       |          The Native window API that with wrapper and encapsulation          |      stable       |

##

<p>&nbsp;
</p>

<p align="center">
<a href="https://github.com/Caishangqi/Eurekiel/issues">
<img src="https://i.imgur.com/qPmjSXy.png" width="160" />
</a> 
<a href="https://github.com/Caishangqi/Eurekiel">
<img src="https://i.imgur.com/L1bU9mr.png" width="160" />
</a>
<a href="[https://discord.gg/3rPcYrPnAs](https://discord.gg/3rPcYrPnAs)">
<img src="https://i.imgur.com/uf6V9ZX.png" width="160" />
</a> 
<a href="https://github.com/Caishangqi">
<img src="https://i.imgur.com/fHQ45KR.png" width="227" />
</a>
</p>

<h1></h1>
<h4 align="center">Find out more about Eurekiel on the <a href="https://github.com/Caishangqi">SMU Pages</a></h4>
<h4 align="center">Looking for the custom support? <a href="https://github.com/Caishangqi">Find it here</a></h4>
