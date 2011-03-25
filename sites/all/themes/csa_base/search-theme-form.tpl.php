<?php
// $Id$

/**
 * @file search-theme-form.tpl.php
 * Theme implementation for displaying a search form.
 */
?>
<div id="search" class="container-inline">
  <input class="search-input form-text" type="text" maxlength="128" name="search_theme_form" id="edit-search_theme_form"  size="20" value="" title="<?php print t('Enter search terms') ?>" />
  <?php print $search['submit']; ?>
  <?php print $search['hidden']; ?>
</div><!-- /search -->