/* Noddy documentation -- sidebar, search filter, prev/next and theme.
 * The navigation itself lives in nav.js (NODDY_NAV); edit that file to add or
 * reorder pages. */
(function () {
  var nav = window.NODDY_NAV || [];
  var here = location.pathname.split('/').pop() || 'index.html';
  if (here.indexOf('.') < 0) here = 'index.html';
  var flat = [];
  nav.forEach(function (sec) { sec.pages.forEach(function (p) { flat.push({ file: p[0], title: p[1], section: sec.title }); }); });

  function store(k, v) { try { if (v === undefined) return localStorage.getItem(k); localStorage.setItem(k, v); } catch (e) { return null; } }

  // Theme
  var saved = store('noddy-theme');
  if (saved) document.documentElement.setAttribute('data-theme', saved);
  var tbtn = document.getElementById('theme-btn');
  if (tbtn) tbtn.addEventListener('click', function () {
    var cur = document.documentElement.getAttribute('data-theme');
    if (!cur) cur = window.matchMedia('(prefers-color-scheme: dark)').matches ? 'dark' : 'light';
    var next = cur === 'dark' ? 'light' : 'dark';
    document.documentElement.setAttribute('data-theme', next);
    store('noddy-theme', next);
  });

  // Sidebar
  var side = document.getElementById('sidebar');
  if (side) {
    var html = '';
    nav.forEach(function (sec) {
      var open = sec.pages.some(function (p) { return p[0] === here; });
      html += '<details' + (open ? ' open' : '') + '><summary>' + sec.title + '</summary><ul>';
      sec.pages.forEach(function (p) {
        html += '<li data-t="' + (p[1] + ' ' + sec.title).toLowerCase() + '"><a href="' + p[0] + '"' +
          (p[0] === here ? ' class="current" aria-current="page"' : '') + '>' + p[1] + '</a></li>';
      });
      html += '</ul></details>';
    });
    html += '<div class="no-hits" hidden>No matching pages.</div>';
    side.innerHTML = html;
    var cur = side.querySelector('a.current');
    if (cur && cur.scrollIntoView) cur.scrollIntoView({ block: 'center' });
  }

  // Search filter (page titles)
  var q = document.getElementById('search');
  if (q && side) q.addEventListener('input', function () {
    var v = q.value.trim().toLowerCase(), any = false;
    side.querySelectorAll('details').forEach(function (d) {
      var hits = 0;
      d.querySelectorAll('li').forEach(function (li) {
        var ok = !v || li.getAttribute('data-t').indexOf(v) >= 0;
        li.classList.toggle('hidden', !ok); if (ok) hits++;
      });
      d.style.display = hits ? '' : 'none';
      if (v && hits) d.open = true;
      any = any || hits > 0;
    });
    side.querySelector('.no-hits').hidden = any;
  });
  if (q) q.addEventListener('keydown', function (e) {
    if (e.key === 'Enter') { var a = side.querySelector('li:not(.hidden) a'); if (a) location.href = a.getAttribute('href'); }
  });

  // Mobile drawer
  var mb = document.getElementById('menu-btn');
  if (mb) mb.addEventListener('click', function () { document.body.classList.toggle('nav-open'); });

  // Breadcrumb + prev/next
  var i = -1;
  flat.forEach(function (p, k) { if (p.file === here) i = k; });
  var crumbs = document.getElementById('crumbs');
  if (crumbs && i >= 0) crumbs.textContent = flat[i].section;
  var pager = document.getElementById('pager');
  if (pager && i >= 0) {
    var h = '';
    if (i > 0) h += '<a class="prev" href="' + flat[i - 1].file + '"><span class="dir">&larr; Previous</span>' + flat[i - 1].title + '</a>';
    else h += '<span></span>';
    if (i < flat.length - 1) h += '<a class="next" href="' + flat[i + 1].file + '"><span class="dir">Next &rarr;</span>' + flat[i + 1].title + '</a>';
    pager.innerHTML = h;
  }
})();
