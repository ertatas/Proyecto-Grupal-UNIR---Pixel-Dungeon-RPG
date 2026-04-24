# TASK.md — DungeonJuego
*Última actualización: 24/04/2026*

---

## Estado actual del proyecto

### ✅ Completado

- **Editor de mapas integrado** (F1) — pintura de tiles, guardado/cargado automático
- **Sistema de mapa por capas** — 7 capas independientes
- **Variantes visuales automáticas** — paredes N/S/E/O y puertas H/V
- **Texturas de pared** — PNGs reales en capa base
- **Props decorativos** — candelabros/lámparas y charcos/manchas/huesos
- **Fog of War por zonas** — BFS desde el jugador
- **Sistema de llaves y puertas** — lógica completa + mensajes al HUD
- **HUD — estructura base** — dos paneles en coordenadas de pantalla fijas, independientes de zoom y cámara
- **HUD — panel stats** — retrato (placeholder morado 48×48), vida (100/100), icono llave (placeholder amarillo 24×24), contador llaves. Fuente PressStart2P 12px
- **HUD — consola de mensajes** — word-wrap automático, 8 líneas visibles, fade-out, panel 500×160px, fuente PressStart2P 8px
- **HUD — fuente** — carga `PressStart2P-Regular.ttf` desde `assets/fonts/` con fallback multi-ruta
- **HUD — oculto en editor** — el HUD desaparece al abrir el editor (F1) y se restaura al cerrarlo

---

## ⚠️ HUD — PARCIALMENTE COMPLETO

El HUD funciona y es estable, pero **falta conectarlo con los sistemas futuros**:

| Pendiente | Descripción |
|---|---|
| **Vida dinámica** | `jugador->vida` existe pero siempre vale 100 — no hay sistema de daño |
| **Retrato PNG** | Muestra placeholder morado hasta que arte entregue `retrato_heroe.png` (48×48) |
| **Icono llave PNG** | Muestra placeholder amarillo hasta que arte entregue `llave_icon.png` (24×24) |
| **Vida dinámica** | `jugador->vida` existe (getter `getVida()`/`getVidaMax()`) pero siempre vale 100 — conectar cuando se implemente combate |

---

## 🗓 Backlog — Próximas sesiones

| Prioridad | Tarea |
|---|---|
| Alta | **Escenas de presentación** — pantalla título/intro, fade in/out, texto animado |
| Alta | **Sistema de combate** — daño al jugador y enemigos, conectar `vida` con el HUD |
| Media | **Enemigos con IA básica** — movimiento, detección del jugador |
| Media | **Barra de vida visual** — iconos corazón o barra en panel superior |
| Baja | **Trampas activas** — daño al pisarlas |
| Baja | **Sonido** — efectos al recoger llave, abrir puerta, recibir daño |

---

## 📁 Assets pendientes — checklist para el equipo de arte

| Archivo | Tamaño | Bloqueante |
|---|---|---|
| `assets/textures/ui/retrato_heroe.png` | **48×48 px** | No — placeholder morado activo |
| `assets/textures/ui/llave_icon.png` | **24×24 px** | No — placeholder amarillo activo |
| `assets/textures/ui/corazon.png` | 16×16 px | No — sin implementar aún |
| `assets/textures/tiles/base/suelo.png` | 64×64 px | No — placeholder gris activo |
| `assets/textures/tiles/props/pared/lampara.png` | 16×24 px | No |
| `assets/textures/tiles/props/pared/antorcha.png` | 16×24 px | No |
| `assets/textures/tiles/props/suelo/charco.png` | 48×48 px | No |
| `assets/textures/tiles/props/suelo/mancha.png` | 48×48 px | No |
| `assets/textures/tiles/props/suelo/huesos.png` | 48×48 px | No |

> Los assets de pared (`pared_sur.png`, `pared_norte.png`, `pared_este.png`, `pared_default.png`) ya están entregados e integrados.
