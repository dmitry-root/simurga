
function set_orientation()
{
	if ($(window).width() < $(window).height() * 3/2)
	{
		$('body').removeClass('horz');
		$('body').addClass('vert');
	}
	else
	{
		$('body').removeClass('vert');
		$('body').addClass('horz');
	}
}

$(window).resize(set_orientation);
$(window).load(set_orientation);

$(window).load(function() {
	if ($('.model-photo-wrapper').length == 0)
		return;

	var big_image = $('.model-photo-wrapper > .photo img')[0];
	var big_ref = $('.model-photo-wrapper > .photo > a')[0];

	var previews = $('.model-photo-wrapper > .previews img');
	var previews_ref = $('.model-photo-wrapper > .previews > a');

	$(big_ref).click(function() { return false; });

	// Preload images.
	previews_ref.each(function(index, ref) {
		$('<img/>')[0].src = ref.href;
		$(ref).click(function() {
			big_image.src = ref.href;
			big_ref.href = ref.href;
			return false;
		});
	});
});

$(document).ready(function() {
	$('.lookbook').slippry({
		'transition': 'horizontal',
		'slideMargin': 5,
		'captions': false,
		'speed': 2000,
		'pause': 4000,
		'pager': false
	});
});
