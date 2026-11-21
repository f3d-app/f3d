# Licences des final shaders

F3D est distribué sous licence **BSD-3-Clause** (voir `LICENSE.md` à la racine).
Les shaders de ce dossier ne sont pas tous compatibles avec cette licence.

## Shaders libres — BSD-3-Clause (F3D)

Créations originales des auteurs du projet.

| Shader | Auteur |
|---|---|
| `aura.glsl` | Palpatoche8 |
| `bloom.glsl` | lance-leau |
| `comic.glsl` | Kae712635 |
| `gray.glsl` | mno |
| `negatif.glsl` | lance-leau |
| `no_shader.glsl` | lance-leau |
| `noir_et_blanc.glsl` | lance-leau |
| `sepia.glsl` | mno |
| `temporal_echo.glsl` | ThoomasGM |

## Shaders sous MIT — compatibles avec BSD-3

Adaptés de code sous licence MIT : compatible avec BSD-3, à condition de conserver la
notice de copyright et le texte de la licence MIT (présents en en-tête du shader).

| Shader | Adapté par | Source | Auteur original |
|---|---|---|---|
| `dithering.glsl` | Erwan Le Grand | https://blog.maximeheckel.com/posts/the-art-of-dithering-and-retro-shading-web/ (code : https://github.com/MaximeHeckel/blog.maximeheckel.com, `core/components/MDX/Widgets/Retro/pixel.ts`) | Maxime Heckel |

## Shaders sous CC BY-NC-SA 3.0 — ⚠️ incompatibles avec BSD-3

Adaptés de shaders Shadertoy dont l'auteur n'a pas précisé de licence : la licence par
défaut de Shadertoy s'applique, **Creative Commons BY-NC-SA 3.0**
(attribution obligatoire, pas d'usage commercial, redistribution sous la même licence).

| Shader | Adapté par | Source | Auteur original |
|---|---|---|---|
| `crt.glsl` | Jules Caron | https://www.shadertoy.com/view/WsVSzV | kbjwes77 |
| `halftone.glsl` | Kae712635 | https://www.shadertoy.com/view/4sBBDK (basé sur https://www.shadertoy.com/view/lsSfWV) | starea (Ruofei Du) |
| `kamisuki.glsl` | ThoomasGM | https://www.shadertoy.com/view/7clyWr | iY0Yi (Shadertoy) |
| `old_movie.glsl` | mno | https://www.shadertoy.com/view/mslfD8 | picolocolo |
| `radial.glsl` | mno | https://www.shadertoy.com/view/XsKGRW | Shane (inspiré de Passion, IQ, mu6k) |
| `sigmoid.glsl` | mno | https://www.shadertoy.com/view/MtX3z2 | Loadus |
| `sobel.glsl` | mno | https://www.shadertoy.com/view/Xdf3Rf | Jeroen Baert |
| `water.glsl` | Jules Caron | https://www.shadertoy.com/view/MdlXz8 | David Hoskins (effet original de joltz0r, GLSL Sandbox) |

Ces shaders ne peuvent pas être redistribués sous BSD-3 en l'état. Pour une contribution
à F3D, il faut soit obtenir l'accord de l'auteur original pour une licence permissive
(MIT/BSD), soit réécrire le shader sans reprendre le code original, soit le retirer.
