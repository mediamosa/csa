<?php
// $Id$

/**
 * @file box.tpl.php
 * Theme implementation to display a single box.
 */
?>
<div class="box">
  <?php if ($title): ?>
  <h2><?php print $title; ?></h2>
  <?php endif; ?>

  <div class="content">
    <?php print $content; ?>
  </div>
</div>
