<?php

/**
 * @file
 * Default theme implementation to display an election candidate.
 */

?>
<div id="election-<?php print $election->election_id; ?>-candidate-<?php print $candidate->candidate_id; ?>" class="<?php print $classes; ?> clearfix"<?php print $attributes; ?>>

  <?php if (!$page): ?>
    <h2<?php print $title_attributes; ?>><a href="<?php print $url; ?>"><?php print $full_name; ?></a></h2>
  <?php endif; ?>

  <div class="pseudo-field candidate-election">
    <span class="label">Election: </span><span class="item"><?php print $election_link; ?></span>
  </div>

  <div class="pseudo-field candidate-post">
    <span class="label">Standing for: </span><span class="item"><?php print $post_link; ?></span>
  </div>

  <div class="content"<?php print $content_attributes; ?>>
    <?php print render($content); ?>
  </div>

</div>