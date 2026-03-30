/**
 * Web Asset Minimization & Compression
 * 
 * This script minifies index.html and generates a C++ header (web_page.h) for embedding in firmware.
 * 
 * IMPORTANT: After any updates to timezone UI in index.html (timezone selectors, config display, etc.),
 * you MUST re-run this script to regenerate the embedded web page header:
 * 
 *   npm run minify
 *   # or manually:
 *   node minimize.js
 * 
 * Failure to regenerate will cause the firmware to serve stale UI without timezone profiling.
 * 
 * Related Tasks:
 *  - T004: Add web asset regeneration step note
 *  - T031-T032: Implement timezone profile selector and config sync in index.html
 *  - T033: Regenerate compressed web payload header after UI changes
 */

import { minify } from 'html-minifier';
import fs from 'fs';

try {
    console.log('Minimizing ./index.html');
    const data = fs.readFileSync('./index.html', 'utf8');
    let result = minify(data, {
        removeAttributeQuotes: true,
        collapseWhitespace: true,
        removeComments: true,
        removeRedundantAttributes: true,
        removeScriptTypeAttributes: true,
        removeStyleLinkTypeAttributes: true,
        removeTagWhitespace: true,
        useShortDoctype: true,
        minifyCSS: true,
        minifyJS: true
    });
    //fs.writeFileSync('./index.min.html', result);

    const page_h = 
`#ifndef WEB_PAGE_H
#define WEB_PAGE_H
#define WEB_PAGE "${result.replaceAll(`"`, `\\"`)}"
#endif`

    fs.writeFileSync('../include/web_page.h', page_h);
    console.log('Generated ../include/web_page.h');
} catch (err) {
    console.error(err);
}
