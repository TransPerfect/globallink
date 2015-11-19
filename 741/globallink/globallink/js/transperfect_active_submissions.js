(function($) {
  Drupal.behaviors.transperfectActiveSubmissions = {
    attach: function(context, settings) {
      if ($('.transperfect-node-select-submission-form', context).length == 0) {
        return;
      }

      $('.transperfect-node-select-submission-edit', context).change(function() {
        $('.transperfect-node-select-submission-form').submit();
      });
    }
  };
})(jQuery);
