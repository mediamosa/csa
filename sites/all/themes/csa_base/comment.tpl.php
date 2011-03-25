<?php
// $Id$

/**
 * @file comment.tpl.php
 * Theme implementation to display a single comment.
 */
?>
<div class="<?php print $comment_classes; ?>">
  <?php if ($new != ''): ?>
    <span class="new"><?php print $new; ?></span>
  <?php endif; ?>

  <h3 class="title"><?php print $title; ?></h3>

  <?php if ($picture) print $picture; ?>

  <span class="submitted"><?php print t('Submitted on ') . format_date($comment->timestamp, 'custom', 'F jS, Y') . t(' by '); ?> <?php print theme('username', $comment); ?></span>

  <div class="content">
    <?php print $content; ?>
  </div>

  <div class="links">
    <?php print $links; ?>
  </div>
</div>
