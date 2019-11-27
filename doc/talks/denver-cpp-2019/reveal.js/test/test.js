// These tests expect the DOM to contain a presentation
// with the following slide structure:
//
// 1
// 2 - Three sub-slides
// 3 - Three fragment elements
// 3 - Two fragments with same data-fragment-index
// 4

Reveal.addEventListener( 'ready', function() {

	// ---------------------------------------------------------------
	// DOM TESTS

	QUnit.module( 'DOM' );

	QUnit.test( 'Initial slides classes', function( assert ) {
		var horizontalSlides = document.querySelectorAll( '.reveal .slides>section' )

		assert.strictEqual( document.querySelectorAll( '.reveal .slides section.past' ).length, 0, 'no .past slides' );
		assert.strictEqual( document.querySelectorAll( '.reveal .slides section.present' ).length, 1, 'one .present slide' );
		assert.strictEqual( document.querySelectorAll( '.reveal .slides>section.future' ).length, horizontalSlides.length - 1, 'remaining horizontal slides are .future' );

		assert.strictEqual( document.querySelectorAll( '.reveal .slides section.stack' ).length, 2, 'two .stacks' );

		assert.ok( document.querySelectorAll( '.reveal .slides section.stack' )[0].querySelectorAll( '.future' ).length > 0, 'vertical slides are given .future' );
	});

	// ---------------------------------------------------------------
	// API TESTS

	QUnit.module( 'API' );

	QUnit.test( 'Reveal.isReady', function( assert ) {
		assert.strictEqual( Reveal.isReady(), true, 'returns true' );
	});

	QUnit.test( 'Reveal.isOverview', function( assert ) {
		assert.strictEqual( Reveal.isOverview(), false, 'false by default' );

		Reveal.toggleOverview();
		assert.strictEqual( Reveal.isOverview(), true, 'true after toggling on' );

		Reveal.toggleOverview();
		assert.strictEqual( Reveal.isOverview(), false, 'false after toggling off' );
	});

	QUnit.test( 'Reveal.isPaused', function( assert ) {
		assert.strictEqual( Reveal.isPaused(), false, 'false by default' );

		Reveal.togglePause();
		assert.strictEqual( Reveal.isPaused(), true, 'true after pausing' );

		Reveal.togglePause();
		assert.strictEqual( Reveal.isPaused(), false, 'false after resuming' );
	});

	QUnit.test( 'Reveal.isFirstSlide', function( assert ) {
		Reveal.slide( 0, 0 );
		assert.strictEqual( Reveal.isFirstSlide(), true, 'true after Reveal.slide( 0, 0 )' );

		Reveal.slide( 1, 0 );
		assert.strictEqual( Reveal.isFirstSlide(), false, 'false after Reveal.slide( 1, 0 )' );

		Reveal.slide( 0, 0 );
		assert.strictEqual( Reveal.isFirstSlide(), true, 'true after Reveal.slide( 0, 0 )' );
	});

	QUnit.test( 'Reveal.isFirstSlide after vertical slide', function( assert ) {
		Reveal.slide( 1, 1 );
		Reveal.slide( 0, 0 );
		assert.strictEqual( Reveal.isFirstSlide(), true, 'true after Reveal.slide( 1, 1 ) and then Reveal.slide( 0, 0 )' );
	});

	QUnit.test( 'Reveal.isLastSlide', function( assert ) {
		Reveal.slide( 0, 0 );
		assert.strictEqual( Reveal.isLastSlide(), false, 'false after Reveal.slide( 0, 0 )' );

		var lastSlideIndex = document.querySelectorAll( '.reveal .slides>section' ).length - 1;

		Reveal.slide( lastSlideIndex, 0 );
		assert.strictEqual( Reveal.isLastSlide(), true, 'true after Reveal.slide( '+ lastSlideIndex +', 0 )' );

		Reveal.slide( 0, 0 );
		assert.strictEqual( Reveal.isLastSlide(), false, 'false after Reveal.slide( 0, 0 )' );
	});

	QUnit.test( 'Reveal.isLastSlide after vertical slide', function( assert ) {
		var lastSlideIndex = document.querySelectorAll( '.reveal .slides>section' ).length - 1;

		Reveal.slide( 1, 1 );
		Reveal.slide( lastSlideIndex );
		assert.strictEqual( Reveal.isLastSlide(), true, 'true after Reveal.slide( 1, 1 ) and then Reveal.slide( '+ lastSlideIndex +', 0 )' );
	});

	QUnit.test( 'Reveal.getTotalSlides', function( assert ) {
		assert.strictEqual( Reveal.getTotalSlides(), 8, 'eight slides in total' );
	});

	QUnit.test( 'Reveal.getIndices', function( assert ) {
		var indices = Reveal.getIndices();

		assert.ok( indices.hasOwnProperty( 'h' ), 'h exists' );
		assert.ok( indices.hasOwnProperty( 'v' ), 'v exists' );
		assert.ok( indices.hasOwnProperty( 'f' ), 'f exists' );

		Reveal.slide( 1, 0 );
		assert.strictEqual( Reveal.getIndices().h, 1, 'h 1' );
		assert.strictEqual( Reveal.getIndices().v, 0, 'v 0' );

		Reveal.slide( 1, 2 );
		assert.strictEqual( Reveal.getIndices().h, 1, 'h 1' );
		assert.strictEqual( Reveal.getIndices().v, 2, 'v 2' );

		Reveal.slide( 0, 0 );
		assert.strictEqual( Reveal.getIndices().h, 0, 'h 0' );
		assert.strictEqual( Reveal.getIndices().v, 0, 'v 0' );
	});

	QUnit.test( 'Reveal.getSlide', function( assert ) {
		assert.equal( Reveal.getSlide( 0 ), document.querySelector( '.reveal .slides>section:first-child' ), 'gets correct first slide' );
		assert.equal( Reveal.getSlide( 1 ), document.querySelector( '.reveal .slides>section:nth-child(2)' ), 'no v index returns stack' );
		assert.equal( Reveal.getSlide( 1, 0 ), document.querySelector( '.reveal .slides>section:nth-child(2)>section:nth-child(1)' ), 'v index 0 returns first vertical child' );
		assert.equal( Reveal.getSlide( 1, 1 ), document.querySelector( '.reveal .slides>section:nth-child(2)>section:nth-child(2)' ), 'v index 1 returns second vertical child' );

		assert.strictEqual( Reveal.getSlide( 100 ), undefined, 'undefined when out of horizontal bounds' );
		assert.strictEqual( Reveal.getSlide( 1, 100 ), undefined, 'undefined when out of vertical bounds' );
	});

	QUnit.test( 'Reveal.getSlideBackground', function( assert ) {
		assert.equal( Reveal.getSlideBackground( 0 ), document.querySelector( '.reveal .backgrounds>.slide-background:first-child' ), 'gets correct first background' );
		assert.equal( Reveal.getSlideBackground( 1 ), document.querySelector( '.reveal .backgrounds>.slide-background:nth-child(2)' ), 'no v index returns stack' );
		assert.equal( Reveal.getSlideBackground( 1, 0 ), document.querySelector( '.reveal .backgrounds>.slide-background:nth-child(2) .slide-background:nth-child(2)' ), 'v index 0 returns first vertical child' );
		assert.equal( Reveal.getSlideBackground( 1, 1 ), document.querySelector( '.reveal .backgrounds>.slide-background:nth-child(2) .slide-background:nth-child(3)' ), 'v index 1 returns second vertical child' );

		assert.strictEqual( Reveal.getSlideBackground( 100 ), undefined, 'undefined when out of horizontal bounds' );
		assert.strictEqual( Reveal.getSlideBackground( 1, 100 ), undefined, 'undefined when out of vertical bounds' );
	});

	QUnit.test( 'Reveal.getSlideNotes', function( assert ) {
		Reveal.slide( 0, 0 );
		assert.ok( Reveal.getSlideNotes() === 'speaker notes 1', 'works with <aside class="notes">' );

		Reveal.slide( 1, 0 );
		assert.ok( Reveal.getSlideNotes() === 'speaker notes 2', 'works with <section data-notes="">' );
	});

	QUnit.test( 'Reveal.getPreviousSlide/getCurrentSlide', function( assert ) {
		Reveal.slide( 0, 0 );
		Reveal.slide( 1, 0 );

		var firstSlide = document.querySelector( '.reveal .slides>section:first-child' );
		var secondSlide = document.querySelector( '.reveal .slides>section:nth-child(2)>section' );

		assert.equal( Reveal.getPreviousSlide(), firstSlide, 'previous is slide #0' );
		assert.equal( Reveal.getCurrentSlide(), secondSlide, 'current is slide #1' );
	});

	QUnit.test( 'Reveal.getProgress', function( assert ) {
		Reveal.slide( 0, 0 );
		assert.strictEqual( Reveal.getProgress(), 0, 'progress is 0 on first slide' );

		var lastSlideIndex = document.querySelectorAll( '.reveal .slides>section' ).length - 1;

		Reveal.slide( lastSlideIndex, 0 );
		assert.strictEqual( Reveal.getProgress(), 1, 'progress is 1 on last slide' );
	});

	QUnit.test( 'Reveal.getScale', function( assert ) {
		assert.ok( typeof Reveal.getScale() === 'number', 'has scale' );
	});

	QUnit.test( 'Reveal.getConfig', function( assert ) {
		assert.ok( typeof Reveal.getConfig() === 'object', 'has config' );
	});

	QUnit.test( 'Reveal.configure', function( assert ) {
		assert.strictEqual( Reveal.getConfig().loop, false, '"loop" is false to start with' );

		Reveal.configure({ loop: true });
		assert.strictEqual( Reveal.getConfig().loop, true, '"loop" has changed to true' );

		Reveal.configure({ loop: false, customTestValue: 1 });
		assert.strictEqual( Reveal.getConfig().customTestValue, 1, 'supports custom values' );
	});

	QUnit.test( 'Reveal.availableRoutes', function( assert ) {
		Reveal.slide( 0, 0 );
		assert.deepEqual( Reveal.availableRoutes(), { left: false, up: false, down: false, right: true }, 'correct for first slide' );

		Reveal.slide( 1, 0 );
		assert.deepEqual( Reveal.availableRoutes(), { left: true, up: false, down: true, right: true }, 'correct for vertical slide' );
	});

	QUnit.test( 'Reveal.next', function( assert ) {
		Reveal.slide( 0, 0 );

		// Step through vertical child slides
		Reveal.next();
		assert.deepEqual( Reveal.getIndices(), { h: 1, v: 0, f: undefined } );

		Reveal.next();
		assert.deepEqual( Reveal.getIndices(), { h: 1, v: 1, f: undefined } );

		Reveal.next();
		assert.deepEqual( Reveal.getIndices(), { h: 1, v: 2, f: undefined } );

		// Step through fragments
		Reveal.next();
		assert.deepEqual( Reveal.getIndices(), { h: 2, v: 0, f: -1 } );

		Reveal.next();
		assert.deepEqual( Reveal.getIndices(), { h: 2, v: 0, f: 0 } );

		Reveal.next();
		assert.deepEqual( Reveal.getIndices(), { h: 2, v: 0, f: 1 } );

		Reveal.next();
		assert.deepEqual( Reveal.getIndices(), { h: 2, v: 0, f: 2 } );
	});

	QUnit.test( 'Reveal.next at end', function( assert ) {
		Reveal.slide( 3 );

		// We're at the end, this should have no effect
		Reveal.next();
		assert.deepEqual( Reveal.getIndices(), { h: 3, v: 0, f: undefined } );

		Reveal.next();
		assert.deepEqual( Reveal.getIndices(), { h: 3, v: 0, f: undefined } );
	});


	// ---------------------------------------------------------------
	// FRAGMENT TESTS

	QUnit.module( 'Fragments' );

	QUnit.test( 'Sliding to fragments', function( assert ) {
		Reveal.slide( 2, 0, -1 );
		assert.deepEqual( Reveal.getIndices(), { h: 2, v: 0, f: -1 }, 'Reveal.slide( 2, 0, -1 )' );

		Reveal.slide( 2, 0, 0 );
		assert.deepEqual( Reveal.getIndices(), { h: 2, v: 0, f: 0 }, 'Reveal.slide( 2, 0, 0 )' );

		Reveal.slide( 2, 0, 2 );
		assert.deepEqual( Reveal.getIndices(), { h: 2, v: 0, f: 2 }, 'Reveal.slide( 2, 0, 2 )' );

		Reveal.slide( 2, 0, 1 );
		assert.deepEqual( Reveal.getIndices(), { h: 2, v: 0, f: 1 }, 'Reveal.slide( 2, 0, 1 )' );
	});

	QUnit.test( 'Hiding all fragments', function( assert ) {
		var fragmentSlide = document.querySelector( '#fragment-slides>section:nth-child(1)' );

		Reveal.slide( 2, 0, 0 );
		assert.strictEqual( fragmentSlide.querySelectorAll( '.fragment.visible' ).length, 1, 'one fragment visible when index is 0' );

		Reveal.slide( 2, 0, -1 );
		assert.strictEqual( fragmentSlide.querySelectorAll( '.fragment.visible' ).length, 0, 'no fragments visible when index is -1' );
	});

	QUnit.test( 'Current fragment', function( assert ) {
		var fragmentSlide = document.querySelector( '#fragment-slides>section:nth-child(1)' );
		var lastFragmentIndex = [].slice.call( fragmentSlide.querySelectorAll( '.fragment' ) ).pop().getAttribute( 'data-fragment-index' );

		Reveal.slide( 2, 0 );
		assert.strictEqual( fragmentSlide.querySelectorAll( '.fragment.current-fragment' ).length, 0, 'no current fragment at index -1' );

		Reveal.slide( 2, 0, 0 );
		assert.strictEqual( fragmentSlide.querySelectorAll( '.fragment.current-fragment' ).length, 1, 'one current fragment at index 0' );

		Reveal.slide( 1, 0, 0 );
		assert.strictEqual( fragmentSlide.querySelectorAll( '.fragment.current-fragment' ).length, 0, 'no current fragment when navigating to previous slide' );

		Reveal.slide( 3, 0, 0 );
		assert.strictEqual( fragmentSlide.querySelectorAll( '.fragment.current-fragment' ).length, 0, 'no current fragment when navigating to next slide' );

		Reveal.slide( 2, 1, -1 );
		Reveal.prev();
		assert.strictEqual( fragmentSlide.querySelector( '.fragment.current-fragment' ).getAttribute( 'data-fragment-index' ), lastFragmentIndex, 'last fragment is current fragment when returning from future slide' );
	});

	QUnit.test( 'Stepping through fragments', function( assert ) {
		Reveal.slide( 2, 0, -1 );

		// forwards:

		Reveal.next();
		assert.deepEqual( Reveal.getIndices(), { h: 2, v: 0, f: 0 }, 'next() goes to next fragment' );

		Reveal.right();
		assert.deepEqual( Reveal.getIndices(), { h: 2, v: 0, f: 1 }, 'right() goes to next fragment' );

		Reveal.down();
		assert.deepEqual( Reveal.getIndices(), { h: 2, v: 0, f: 2 }, 'down() goes to next fragment' );

		Reveal.down(); // moves to f #3

		// backwards:

		Reveal.prev();
		assert.deepEqual( Reveal.getIndices(), { h: 2, v: 0, f: 2 }, 'prev() goes to prev fragment' );

		Reveal.left();
		assert.deepEqual( Reveal.getIndices(), { h: 2, v: 0, f: 1 }, 'left() goes to prev fragment' );

		Reveal.up();
		assert.deepEqual( Reveal.getIndices(), { h: 2, v: 0, f: 0 }, 'up() goes to prev fragment' );
	});

	QUnit.test( 'Stepping past fragments', function( assert ) {
		var fragmentSlide = document.querySelector( '#fragment-slides>section:nth-child(1)' );

		Reveal.slide( 0, 0, 0 );
		assert.equal( fragmentSlide.querySelectorAll( '.fragment.visible' ).length, 0, 'no fragments visible when on previous slide' );

		Reveal.slide( 3, 0, 0 );
		assert.equal( fragmentSlide.querySelectorAll( '.fragment.visible' ).length, 3, 'all fragments visible when on future slide' );
	});

	QUnit.test( 'Fragment indices', function( assert ) {
		var fragmentSlide = document.querySelector( '#fragment-slides>section:nth-child(2)' );

		Reveal.slide( 3, 0, 0 );
		assert.equal( fragmentSlide.querySelectorAll( '.fragment.visible' ).length, 2, 'both fragments of same index are shown' );

		// This slide has three fragments, first one is index 0, second and third have index 1
		Reveal.slide( 2, 2, 0 );
		assert.equal( Reveal.getIndices().f, 0, 'returns correct index for first fragment' );

		Reveal.slide( 2, 2, 1 );
		assert.equal( Reveal.getIndices().f, 1, 'returns correct index for two fragments with same index' );
	});

	QUnit.test( 'Index generation', function( assert ) {
		var fragmentSlide = document.querySelector( '#fragment-slides>section:nth-child(1)' );

		// These have no indices defined to start with
		assert.equal( fragmentSlide.querySelectorAll( '.fragment' )[0].getAttribute( 'data-fragment-index' ), '0' );
		assert.equal( fragmentSlide.querySelectorAll( '.fragment' )[1].getAttribute( 'data-fragment-index' ), '1' );
		assert.equal( fragmentSlide.querySelectorAll( '.fragment' )[2].getAttribute( 'data-fragment-index' ), '2' );
	});

	QUnit.test( 'Index normalization', function( assert ) {
		var fragmentSlide = document.querySelector( '#fragment-slides>section:nth-child(3)' );

		// These start out as 1-4-4 and should normalize to 0-1-1
		assert.equal( fragmentSlide.querySelectorAll( '.fragment' )[0].getAttribute( 'data-fragment-index' ), '0' );
		assert.equal( fragmentSlide.querySelectorAll( '.fragment' )[1].getAttribute( 'data-fragment-index' ), '1' );
		assert.equal( fragmentSlide.querySelectorAll( '.fragment' )[2].getAttribute( 'data-fragment-index' ), '1' );
	});

	QUnit.test( 'fragmentshown event', function( assert ) {
		assert.expect( 2 );
		var done = assert.async( 2 );

		var _onEvent = function( event ) {
			assert.ok( true, 'event fired' );
			done();
		}

		Reveal.addEventListener( 'fragmentshown', _onEvent );

		Reveal.slide( 2, 0 );
		Reveal.slide( 2, 0 ); // should do nothing
		Reveal.slide( 2, 0, 0 ); // should do nothing
		Reveal.next();
		Reveal.next();
		Reveal.prev(); // shouldn't fire fragmentshown

		Reveal.removeEventListener( 'fragmentshown', _onEvent );
	});

	QUnit.test( 'fragmenthidden event', function( assert ) {
		assert.expect( 2 );
		var done = assert.async( 2 );

		var _onEvent = function( event ) {
			assert.ok( true, 'event fired' );
			done();
		}

		Reveal.addEventListener( 'fragmenthidden', _onEvent );

		Reveal.slide( 2, 0, 2 );
		Reveal.slide( 2, 0, 2 ); // should do nothing
		Reveal.prev();
		Reveal.prev();
		Reveal.next(); // shouldn't fire fragmenthidden

		Reveal.removeEventListener( 'fragmenthidden', _onEvent );
	});


	// ---------------------------------------------------------------
	// AUTO-SLIDE TESTS

	QUnit.module( 'Auto Sliding' );

	QUnit.test( 'Reveal.isAutoSliding', function( assert ) {
		assert.strictEqual( Reveal.isAutoSliding(), false, 'false by default' );

		Reveal.configure({ autoSlide: 10000 });
		assert.strictEqual( Reveal.isAutoSliding(), true, 'true after starting' );

		Reveal.configure({ autoSlide: 0 });
		assert.strictEqual( Reveal.isAutoSliding(), false, 'false after setting to 0' );
	});

	QUnit.test( 'Reveal.toggleAutoSlide', function( assert ) {
		Reveal.configure({ autoSlide: 10000 });

		Reveal.toggleAutoSlide();
		assert.strictEqual( Reveal.isAutoSliding(), false, 'false after first toggle' );
		Reveal.toggleAutoSlide();
		assert.strictEqual( Reveal.isAutoSliding(), true, 'true after second toggle' );

		Reveal.configure({ autoSlide: 0 });
	});

	QUnit.test( 'autoslidepaused', function( assert ) {
		assert.expect( 1 );
		var done = assert.async();

		var _onEvent = function( event ) {
			assert.ok( true, 'event fired' );
			done();
		}

		Reveal.addEventListener( 'autoslidepaused', _onEvent );
		Reveal.configure({ autoSlide: 10000 });
		Reveal.toggleAutoSlide();

		// cleanup
		Reveal.configure({ autoSlide: 0 });
		Reveal.removeEventListener( 'autoslidepaused', _onEvent );
	});

	QUnit.test( 'autoslideresumed', function( assert ) {
		assert.expect( 1 );
		var done = assert.async();

		var _onEvent = function( event ) {
			assert.ok( true, 'event fired' );
			done();
		}

		Reveal.addEventListener( 'autoslideresumed', _onEvent );
		Reveal.configure({ autoSlide: 10000 });
		Reveal.toggleAutoSlide();
		Reveal.toggleAutoSlide();

		// cleanup
		Reveal.configure({ autoSlide: 0 });
		Reveal.removeEventListener( 'autoslideresumed', _onEvent );
	});


	// ---------------------------------------------------------------
	// CONFIGURATION VALUES

	QUnit.module( 'Configuration' );

	QUnit.test( 'Controls', function( assert ) {
		var controlsElement = document.querySelector( '.reveal>.controls' );

		Reveal.configure({ controls: false });
		assert.equal( controlsElement.style.display, 'none', 'controls are hidden' );

		Reveal.configure({ controls: true });
		assert.equal( controlsElement.style.display, 'block', 'controls are visible' );
	});

	QUnit.test( 'Progress', function( assert ) {
		var progressElement = document.querySelector( '.reveal>.progress' );

		Reveal.configure({ progress: false });
		assert.equal( progressElement.style.display, 'none', 'progress are hidden' );

		Reveal.configure({ progress: true });
		assert.equal( progressElement.style.display, 'block', 'progress are visible' );
	});

	QUnit.test( 'Loop', function( assert ) {
		Reveal.configure({ loop: true });

		Reveal.slide( 0, 0 );

		Reveal.left();
		assert.notEqual( Reveal.getIndices().h, 0, 'looped from start to end' );

		Reveal.right();
		assert.equal( Reveal.getIndices().h, 0, 'looped from end to start' );

		Reveal.configure({ loop: false });
	});


	// ---------------------------------------------------------------
	// LAZY-LOADING TESTS

	QUnit.module( 'Lazy-Loading' );

	QUnit.test( 'img with data-src', function( assert ) {
		assert.strictEqual( document.querySelectorAll( '.reveal section img[src]' ).length, 1, 'Image source has been set' );
	});

	QUnit.test( 'video with data-src', function( assert ) {
		assert.strictEqual( document.querySelectorAll( '.reveal section video[src]' ).length, 1, 'Video source has been set' );
	});

	QUnit.test( 'audio with data-src', function( assert ) {
		assert.strictEqual( document.querySelectorAll( '.reveal section audio[src]' ).length, 1, 'Audio source has been set' );
	});

	QUnit.test( 'iframe with data-src', function( assert ) {
		Reveal.slide( 0, 0 );
		assert.strictEqual( document.querySelectorAll( '.reveal section iframe[src]' ).length, 0, 'Iframe source is not set' );
		Reveal.slide( 2, 1 );
		assert.strictEqual( document.querySelectorAll( '.reveal section iframe[src]' ).length, 1, 'Iframe source is set' );
		Reveal.slide( 2, 2 );
		assert.strictEqual( document.querySelectorAll( '.reveal section iframe[src]' ).length, 0, 'Iframe source is not set' );
	});

	QUnit.test( 'background images', function( assert ) {
		var imageSource1 = Reveal.getSlide( 0 ).getAttribute( 'data-background-image' );
		var imageSource2 = Reveal.getSlide( 1, 0 ).getAttribute( 'data-background' );

		// check that the images are applied to the background elements
		assert.ok( Reveal.getSlideBackground( 0 ).querySelector( '.slide-background-content' ).style.backgroundImage.indexOf( imageSource1 ) !== -1, 'data-background-image worked' );
		assert.ok( Reveal.getSlideBackground( 1, 0 ).querySelector( '.slide-background-content' ).style.backgroundImage.indexOf( imageSource2 ) !== -1, 'data-background worked' );
	});


	// ---------------------------------------------------------------
	// EVENT TESTS

	QUnit.module( 'Events' );

	QUnit.test( 'slidechanged', function( assert ) {
		assert.expect( 3 );
		var done = assert.async( 3 );

		var _onEvent = function( event ) {
			assert.ok( true, 'event fired' );
			done();
		}

		Reveal.addEventListener( 'slidechanged', _onEvent );

		Reveal.slide( 1, 0 ); // should trigger
		Reveal.slide( 1, 0 ); // should do nothing
		Reveal.next(); // should trigger
		Reveal.slide( 3, 0 ); // should trigger
		Reveal.next(); // should do nothing

		Reveal.removeEventListener( 'slidechanged', _onEvent );

	});

	QUnit.test( 'paused', function( assert ) {
		assert.expect( 1 );
		var done = assert.async();

		var _onEvent = function( event ) {
			assert.ok( true, 'event fired' );
			done();
		}

		Reveal.addEventListener( 'paused', _onEvent );

		Reveal.togglePause();
		Reveal.togglePause();

		Reveal.removeEventListener( 'paused', _onEvent );
	});

	QUnit.test( 'resumed', function( assert ) {
		assert.expect( 1 );
		var done = assert.async();

		var _onEvent = function( event ) {
			assert.ok( true, 'event fired' );
			done();
		}

		Reveal.addEventListener( 'resumed', _onEvent );

		Reveal.togglePause();
		Reveal.togglePause();

		Reveal.removeEventListener( 'resumed', _onEvent );
	});

} );

Reveal.initialize();
