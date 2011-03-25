<?php
// $Id$

/**
 * @file comment-wrapper.tpl.php
 * Theme implementation to display a comment wrapper.
 */
?>

<?php if ($content) : ?>
<div id="comments" class="comments <?php if (isset($skinr)) { print $skinr; } ?>">
  <div class="comments-inner">
    <h2 class="comments-header"><?php print t('Comments'); ?></h2>

    <div class="content">
      <?php print $content; ?>
    </div>
  </div>
</div>
<?php endif; ?><!-- /comments -->
