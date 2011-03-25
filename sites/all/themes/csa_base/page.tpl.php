<?php
// $Id$

/**
 * @file page.tpl.php
 * Theme implementation to display a single Drupal page.
 *
 * - There should be a better way to add the conditional comments to our page.tpl.php! (SEE FRAMEWORK THEME!)
 */
?>
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN"
    "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html xmlns="http://www.w3.org/1999/xhtml" lang="<?php print $variables['language']->language; ?>" xml:lang="<?php print $variables['language']->language; ?>">
<head>
<!--

      IMPLEMENTATIE DOOR MADCAP BV
    ********************************
           verademing in ICT
            www.madcap.nl

-->
<title><?php print $head_title; ?></title>
<?php print $head; ?>
<?php print $styles; ?>
<?php print $scripts; ?>
</head>

<body class="<?php print $body_classes; ?>">

<div id="container" class="wrapper">
  <div id="header" class="clear-block">
    <div class="header-inner section">
        <?php if (!empty($logo)): ?>
          <div class="site-header site-information"><span class="logo"><?php print $logo;?></span></div>
        <?php endif; ?>
      <?php if (!empty($search_box)): print $search_box; endif; ?>
      <?php if (!empty($header)): ?>
      <div class="site-header region">
        <?php print $header; ?>
        <!--[if lte IE 7]><div class="clear-both"></div><![endif]-->
      </div>
      <?php endif; ?>
      <?php if (!empty($primary_menu)): ?>
      <div id="navigation-primary" class="<?php print theme_get_setting('primary_links_display_style'); ?> site-header">
        <div class="navigation-inner content">
          <?php print $primary_menu; ?>
          <!--[if lte IE 7]><div class="clear-both"></div><![endif]-->
        </div>
      </div><!-- /#navigation-primary -->
      <?php endif; ?>
    </div>
  </div><!-- /#header -->

  <div id="columns" class="clear-block section">
    <?php if (!empty($secondary_menu)): ?>
      <div id="navigation-secondary" class="<?php print theme_get_setting('secondary_links_display_style'); ?>">
       <div class="navigation-inner content">
          <?php if (!empty($secondary_menu)): print $secondary_menu; endif; ?>
         <!--[if lte IE 7]><div class="clear-both"></div><![endif]-->
        </div>
      </div><!-- /#navigation-secondary -->
    <?php endif; ?>
    <div id="content" class="column">
       <div class="content-inner">
        <!--[if lte IE 7]><div class="clear-both"></div><![endif]-->
        <?php if (!empty($breadcrumb)): print $breadcrumb; endif; ?>
        <?php if (!empty($messages)): print $messages; endif; ?>
        <?php if (!empty($mission)): ?>
        <div id="mission" class="content">
          <div class="mission-inner">
            <?php print $mission; ?>
          </div>
        </div><!-- /#mission -->
        <?php endif; ?>
        <?php if (!empty($site_name)): ?><h1 class="sitename"><?php print $site_name; ?></h1><?php endif; ?>
        <?php if (!empty($title)): ?><h2 class="title page-title"><?php print $title; ?></h2><?php endif; ?>
        <div class="clear-both"></div>
        <?php if (!empty($help)): print $help; endif; ?>
        <?php if (!empty($content_top)):?>
        <div id="content-top" class="region">
          <?php print $content_top; ?>
        </div><!-- /#content-top -->
        <?php endif; ?>
        <?php if (!empty($tabs)): ?>
        <div class="tabs">
          <?php print $tabs; ?>
        </div>
        <?php endif; ?>
        <?php if (!empty($content)): print $content; endif; ?>
        <?php if (!empty($content_bottom)): ?>
        <div id="content-bottom" class="region">
          <?php print $content_bottom; ?>
        </div><!--/#content-bottom -->
        <?php endif; ?>
      </div>
    </div><!-- /#content -->
    <?php if (!empty($sidebar_1)): ?>
    <div id="sidebar-1" class="sidebar column region">
      <div class="sidebar-inner">
        <?php print $sidebar_1; ?>
      </div>
    </div><!-- /#sidebar_1 -->
    <?php endif; ?>
    <?php if (!empty($sidebar_2)): ?>
    <div id="sidebar-2" class="sidebar column region">
      <div class="sidebar-inner">
        <?php print $sidebar_2; ?>
      </div>
    </div><!-- /#sidebar_2-->
    <?php endif; ?>
    <!--[if lte IE 7]><div class="clear-both"></div><![endif]-->
  </div><!-- /#columns -->

  <div class="push"></div>
</div><!-- /#container -->

<?php if (!empty($footer) || !empty($footer_message)): ?>
<div id="footer">
  <div class="footer-inner section">
    <?php if (!empty($footer)): ?>
    <div class="site-footer region">
      <?php print $footer; ?>
      <!--[if lte IE 7]><div class="clear-both"></div><![endif]-->
    </div>
    <?php endif; ?>
    <?php if (!empty($footer_message)):?>
    <div class="footer-message">
      <?php print $footer_message; ?>
    </div>
    <?php endif; ?>
  </div>
</div><!-- /#footer -->

<?php endif; ?>
<?php if (!empty($closure) && $closure): print $closure; endif; ?>
</body>
</html>