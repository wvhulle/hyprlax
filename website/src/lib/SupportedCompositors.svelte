<script>
  const compositors = [
    { name: 'Hyprland', status: 'tested', statusText: '✓ Beta', statusColor: 'text-green-400' },
    { name: 'Niri', status: 'experimental', statusText: '◐ Experimental', statusColor: 'text-yellow-400' },
    { name: 'River', status: 'experimental', statusText: '◐ Experimental', statusColor: 'text-yellow-400' },
    { name: 'Sway', status: 'experimental', statusText: '◐ Experimental', statusColor: 'text-yellow-400' },
  ];
</script>

<div class="w-full max-w-4xl mb-8">
  <h2 class="text-xl sm:text-2xl font-mono font-bold text-center mb-6 bg-clip-text text-transparent bg-gradient-to-r from-hypr-blue to-hypr-pink">
    Supported Compositors
  </h2>
  <div class="grid grid-cols-1 sm:grid-cols-2 lg:grid-cols-4 gap-4">
    {#each compositors as compositor}
      <div class="bg-hypr-dark/30 backdrop-blur-xl rounded-xl border border-white/10 p-4 text-center">
        <div class="{compositor.statusColor} font-mono text-sm mb-2">{compositor.statusText}</div>
        <div class="font-mono text-gray-300">{compositor.name}</div>
      </div>
    {/each}
  </div>

</div>