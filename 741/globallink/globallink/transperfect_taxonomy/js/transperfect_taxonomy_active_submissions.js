(function($) {
  Drupal.behaviors.transperfectTaxonomyActiveSubmissions = {
    attach: function(context, settings) {
      if ($('.transperfect-taxonomy-active-select-form', context).length == 0) {
        return;
      }

      $('.transperfect-taxonomy-select-active-submission', context).change(function() {
        $('.transperfect-taxonomy-active-select-form').submit();
      });
    }
  };
})(jQuery);
