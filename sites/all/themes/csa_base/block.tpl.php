<?php
// $Id$

/**
 * @file block.tpl.php
 * Theme implementation to display a single block.
 */
?>
<div id="block-<?php print theme('safe_css_name', $block->module .'-'. $block->delta); ?>" class="<?php print $block_classes ?>" >
  <div class="block-inner">
    <?php if ($block->subject): ?>
    <h2 class="title"><?php print $block->subject; ?></h2>
    <?php endif; ?>

    <div class="content">
      <?php print $block->content; ?>
    </div>
  </div>
</div>
