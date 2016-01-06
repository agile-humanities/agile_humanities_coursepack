<?php
/**
 * @file
 * Templated output.
 */
?>

<div>
    <?php print $search_block; ?>
    <?php print $metadata_link; ?>
    <br />
    <?php print $consolidate_link; ?>
    <br />
    <?php print $clone_link; ?>
    <br />
    <?php print $manage_link; ?>
    <?php print $document; ?>
    <?php print $attribution ? t("Transmogrified from !link", array('!link' => $attribution)) : ''; ?>
    <?php print $links; ?>
</div>