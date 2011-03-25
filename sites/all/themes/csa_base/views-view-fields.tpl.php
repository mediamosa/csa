<?php
// $Id$

/**
 * @file views-view-fields.tpl.php
 * Theme implementation to display a field as a row in a view.
 */
?>
<?php 
  $counter = 0;
  $ni = count($fields);
?>
<?php if ($ni>1):?>
<div class='left-side'>
<?php endif; ?>
<?php foreach ($fields as $id => $field): ?>
  <?php if (!empty($field->separator)): ?>
    <?php print $field->separator; ?>
  <?php endif; ?>
  <<?php print $field->inline_html;?> class="views-field-<?php print $field->class; ?>">
    <?php if ($field->label): ?>
      <label class="views-label-<?php print $field->class; ?>">
        <?php print $field->label; ?>:
      </label>
    <?php endif; ?>
      <?php
      // $field->element_type is either SPAN or DIV depending upon whether or not
      // the field is a 'block' element type or 'inline' element type.
      ?>
      <<?php print $field->element_type; ?> class="field-content"><?php print $field->content; ?></<?php print $field->element_type; ?>>
  </<?php print $field->inline_html;?>>
<?php if ($counter++ == 0 && $ni>1): ?>
</div><div class='right-side'>
<?php endif; ?>
<?php endforeach; ?>
<?php if ($ni>1):?>
</div>
<div class="clear-both"></div>
<?php endif;
