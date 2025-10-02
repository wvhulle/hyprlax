<script>
  import { onMount } from 'svelte';
  export let mouseX = 0.5;
  export let mouseY = 0.5;
  let redditScore = null;
  let redditLoading = true;
  // Import the demo GIF via Vite so it works in dev and build, and respects base path
  import demoGif from '../../../assets/hyprlax-demo.gif?url';

  const fetchRedditScore = async () => {
    try {
      const response = await fetch('https://www.reddit.com/comments/1nh59j7.json');
      
      if (!response.ok) {
        // Silently fail on any error including 429
        redditLoading = false;
        return;
      }
      
      const data = await response.json();
      if (data && data[0] && data[0].data && data[0].data.children && data[0].data.children[0]) {
        redditScore = data[0].data.children[0].data.score;
      }
    } catch (error) {
      // Silently fail on any error
      console.debug('Could not fetch Reddit score:', error);
    } finally {
      redditLoading = false;
    }
  };

  onMount(fetchRedditScore)

</script>

<!-- Logo/Title -->
<div class="mb-8 text-center">
  <h1 class="text-5xl sm:text-6xl md:text-7xl lg:text-8xl font-mono font-bold mb-4">
    <span class="bg-clip-text text-transparent bg-gradient-to-r from-hypr-blue to-hypr-pink">
      hyprlax
    </span>
  </h1>
  <p class="text-gray-400 text-lg sm:text-xl md:text-2xl font-mono">
    smooth parallax wallpapers for hyprland
  </p>
</div>

<!-- Video/Screencast -->
<div class="w-full max-w-4xl aspect-video mb-12 relative group">
  <div class="absolute inset-0 bg-gradient-to-r from-hypr-blue/20 to-hypr-pink/20 rounded-2xl blur-3xl group-hover:blur-2xl transition-all duration-500"></div>
  <div class="relative bg-hypr-dark/80 backdrop-blur-xl rounded-2xl border border-white/10 overflow-hidden shadow-2xl">
    <img 
      src={demoGif} 
      alt="Hyprlax parallax wallpaper animation demo" 
      class="w-full h-full object-cover"
      loading="lazy"
    />
  </div>
</div>

<!-- Announcement -->
  <div class="mb-8">
    <a 
      href="https://www.reddit.com/r/hyprland/comments/1nh59j7/buttery_smooth_parallax_wallpaper_engine_for/" 
      target="_blank"
      rel="noopener noreferrer"
      class="group inline-flex items-center gap-3 px-4 py-2 bg-orange-500/10 border border-orange-500/20 rounded-lg hover:border-orange-500/40 transition-all duration-300"
    >
      <svg class="w-5 h-5 text-orange-500" fill="currentColor" viewBox="0 0 24 24" xmlns="http://www.w3.org/2000/svg">
        <path d="M12 0A12 12 0 0 0 0 12a12 12 0 0 0 12 12 12 12 0 0 0 12-12A12 12 0 0 0 12 0zm5.01 4.744c.688 0 1.25.561 1.25 1.249a1.25 1.25 0 0 1-2.498.056l-2.597-.547-.8 3.747c1.824.07 3.48.632 4.674 1.488.308-.309.73-.491 1.207-.491.968 0 1.754.786 1.754 1.754 0 .716-.435 1.333-1.01 1.614a3.111 3.111 0 0 1 .042.52c0 2.694-3.13 4.87-7.004 4.87-3.874 0-7.004-2.176-7.004-4.87 0-.183.015-.366.043-.534A1.748 1.748 0 0 1 4.028 12c0-.968.786-1.754 1.754-1.754.463 0 .898.196 1.207.49 1.207-.883 2.878-1.43 4.744-1.487l.885-4.182a.342.342 0 0 1 .14-.197.35.35 0 0 1 .238-.042l2.906.617a1.214 1.214 0 0 1 1.108-.701zM9.25 12C8.561 12 8 12.562 8 13.25c0 .687.561 1.248 1.25 1.248.687 0 1.248-.561 1.248-1.249 0-.688-.561-1.249-1.249-1.249zm5.5 0c-.687 0-1.248.561-1.248 1.25 0 .687.561 1.248 1.249 1.248.688 0 1.249-.561 1.249-1.249 0-.687-.562-1.249-1.25-1.249zm-5.466 3.99a.327.327 0 0 0-.231.094.33.33 0 0 0 0 .463c.842.842 2.484.913 2.961.913.477 0 2.105-.056 2.961-.913a.361.361 0 0 0 .029-.463.33.33 0 0 0-.464 0c-.547.533-1.684.73-2.512.73-.828 0-1.979-.196-2.512-.73a.326.326 0 0 0-.232-.095z"/>
      </svg>
      <span class="text-orange-500 font-mono text-sm group-hover:underline">
        announcement post
        {#if redditScore !== null}
          <span class="ml-2 px-2 py-0.5 bg-orange-500/20 rounded text-xs">↑ {redditScore}</span>
        {/if}
      </span>
    </a>
  </div>