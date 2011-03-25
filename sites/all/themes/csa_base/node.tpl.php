<?php
// $Id$

/**
 * @file node.tpl.php
 * Theme implementation to display a single node.
 */
?>
<div id="node-<?php print $node->nid; ?>" class="<?php print $node_classes ?>">
  <?php if ($page == 0): ?>
    <h2 class="title"><a href="<?php print $node_url; ?>"><?php print $title; ?></a></h2>
  <?php endif; ?>

  <?php if ($picture) print $picture; ?>

  <?php if ($submitted): ?>
  <span class="submitted"><?php print $submitted ?></span>
  <?php endif; ?>

  <?php if ($terms): ?>
  <div class="terms">
    <?php print $terms; ?>
  </div>
  <?php endif;?>

  <div class="content">
    <?php print $content; ?>
  </div>

  <?php if ($links): ?>
  <div class="links">
    <?php print $links; ?>
  </div>
  <?php endif; ?>
</div> <!-- /#node-<?php print $node->nid; ?> -->
