function onload(event) {
  if (window.location.pathname.endsWith("/INSTALLATION.html")) {
    enhance_install_page();
  }
}

function enhance_install_page() {
  function open_mac_warning_popup() {
    const popup = document.createElement("div");
    popup.classList = "popup-container bg-grey-dk-200";
    popup.innerHTML = `
        <h2>Warning</h2>
        <p>MacOS package is not signed, if macOS complains that <b>the file is invalid, damaged or corrupted</b>, see the <a href="/doc/user/LIMITATIONS_AND_TROUBLESHOOTING.html#macos">troubleshooting section</a> for a workaround.</p>
    `;

    const button = document.createElement("button");
    button.classList = "btn btn-primary";
    button.innerHTML = "Ok";
    button.addEventListener("click", function () {
      popup.remove();
    });
    popup.append(button);

    document.body.append(popup);
  }

  function retrieve_downloads(target_platform_re) {
    var downloads = [];
    for (const table of document.querySelectorAll("table")) {
      const th = document.querySelector("th");
      if (th) {
        if (/Platform/i.test(th.innerText)) {
          for (const table_row of table.querySelectorAll("tr")) {
            const table_cols = table_row.querySelectorAll("td");
            if (table_cols.length > 1) {
              const link = table_cols[1].querySelector("a");
              if (link) {
                const platform = table_cols[0].innerText;

                if (target_platform_re.test(platform))
                  downloads.push([platform, link.innerText, link.href]);

                if (/MacOS/i.test(platform))
                  link.addEventListener("click", open_mac_warning_popup);
              }
            }
          }
          break; // stop after first relevant table
        }
      }
    }
    return downloads;
  }

  /* check OS: first check for `?os=foo` in the URL, otherwise guess form user agent */
  const params = new URLSearchParams(window.location.search);
  const current_os = params.get("os") || guess_client_OS();

  if (current_os == null) return;

  /* find download links matching the OS */
  const target_platform_re = RegExp(`(${current_os})`, "i");
  const downloads = retrieve_downloads(target_platform_re);

  if (downloads.length < 1) return;

  /* create a box below the first header to insert buttons */
  const header = document.querySelector("h1");
  if (header == null) return;
  const div = document.createElement("div");
  div.setAttribute("class", "install-buttons");
  header.parentNode.insertBefore(div, header.nextSibling);

  /* create download button for each matching link */
  var i = 0;
  for ([platform, filename, url] of downloads) {
    const bolded_platform = platform.replace(target_platform_re, "<b>$1</b>");
    const link = document.createElement("a");
    link.setAttribute("href", url);
    link.setAttribute("class", i++ ? "btn" : "btn btn-primary");
    link.innerHTML = `<div>Get <b>F3D</b> for ${bolded_platform}</div><small>${filename}</small>`;
    if (current_os === "MacOS")
      link.addEventListener("click", open_mac_warning_popup);
    div.append(link);
  }
  const note = document.createElement("div");
  note.innerHTML = "<small>or see other available versions below</small>";
  div.append(note);
}

function guess_client_OS() {
  /* https://stackoverflow.com/a/38241481 */
  const userAgent = window.navigator.userAgent;
  const platform =
    window.navigator?.userAgentData?.platform || window.navigator.platform;
  const macosPlatforms = ["macOS", "Macintosh", "MacIntel", "MacPPC", "Mac68K"];
  const windowsPlatforms = ["Win32", "Win64", "Windows", "WinCE"];
  const iosPlatforms = ["iPhone", "iPad", "iPod"];

  if (macosPlatforms.indexOf(platform) !== -1) {
    return "MacOS";
  } else if (iosPlatforms.indexOf(platform) !== -1) {
    return "iOS";
  } else if (windowsPlatforms.indexOf(platform) !== -1) {
    return "Windows";
  } else if (/Android/.test(userAgent)) {
    return "Android";
  } else if (/Linux/.test(platform)) {
    return "Linux";
  } else {
    return null;
  }
}

window.addEventListener("load", onload);
