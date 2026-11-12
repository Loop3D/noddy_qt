# Noddy documentation

This folder holds the Noddy user guide as a static website: plain HTML pages with one
shared stylesheet and script, no build step. Open `index.html` in a browser to read it
locally, or publish it with GitHub Pages (Settings > Pages > Source: *Deploy from a
branch*, folder `/docs`).

The content is a modernised version of the original Noddy help pages (AMIRA P418
manual, versions 6 and 7), updated for the Qt version of the program: features that
were removed are gone, new features have their own pages, and the screenshots were
retaken from the current program.

## Layout

| Path | Contents |
|------|----------|
| `*.html` | One file per page. Page names keep the original help-file names (e.g. `folds.html` was `folds.htm`) so the program's old help links map directly. |
| `assets/nav.js` | The sidebar contents and page order. Add, rename or reorder pages here. |
| `assets/app.js` | Sidebar, page filter, previous/next links and the dark-mode toggle. |
| `assets/style.css` | All styling, including light and dark colour themes. |
| `images/screens/` | Screenshots of the current program. |
| `images/icons/` | Toolbar icons (copied from `qt_compat/icons/png`). |
| `images/figures/` | Diagrams, equations and test plots from the original manual, plus a few original screenshots that could not be reproduced. |

## Adding a page

1. Copy an existing page, change the `<title>`, the `<h1>` and the content inside `<article>`.
2. Add the file name and title to the right section in `assets/nav.js`.

Useful building blocks inside a page:

```html
<figure class="screen"><img src="images/screens/name.png" alt="..."><figcaption>...</figcaption></figure>
<div class="callout new"><span class="label">New</span>...</div>   <!-- also: callout, callout warn -->
<span class="menu-path">File &gt; Save Picture&hellip;</span>
<img class="ev-icon" src="images/icons/fold.png" alt="fold icon">
```
