<?php
// $Id$

/**
 * @file search-result.tpl.php
 * Theme implementation for displaying a single search result.
 */
?>
<dt class="title">
  <a href="<?php print $url; ?>"><?php print $title; ?></a>
</dt>
<dd>
  <?php if ($snippet) : ?>
    <p class="search-snippet"><?php print $snippet; ?></p>
  <?php endif; ?>
  <?php if ($info) : ?>
  <p class="search-info"><?php print $info; ?></p>
  <?php endif; ?>
</dd>
