<?php
// $Id$

/**
 * @file views-view-grid.tpl.php
 * Theme implementation to display rows in a grid.
 */
?>
<?php if (!empty($title)) : ?>
<h3><?php print $title; ?></h3>
<?php endif; ?>

<div class="views-view-grid views-view-grid-<?php print $options['alignment']; ?>">
  <?php foreach ($rows as $row_number => $columns): ?>
  <?php
  $row_class = 'row-'. ($row_number + 1);
  if ($row_number == 0) {
    $row_class .= ' row-first';
  }
  elseif (count($rows) == ($row_number + 1)) {
    $row_class .= ' row-last';
  }
  ?>
    <div class="row <?php print $row_class; ?> columns-<?php print $options['columns']; ?>">
      <?php foreach ($columns as $column_number => $item): ?>
        <div class="col <?php print 'col-'. ($column_number + 1); ?>">
          <div class="inner">
            <?php print $item; ?>
          </div>
        </div>
      <?php endforeach; ?>        
    </div>
  <?php endforeach; ?> 
</div>
<div class="clear-both"></div>
