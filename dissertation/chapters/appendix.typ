= Appendix

#for size in ("1", "2", "4", "8", "16", "32", "64", "128", "256", "512", "1024", "2048") [
  #figure(
    image("../results/linux/memory/basic_size_"+size+"_memory.png"),
    caption: "Basic benchmark memory usage for size: " + size
  )

  #figure(
    image("../results/linux/time/basic_size_"+size+".0.png"),
    caption: "Basic benchmark for size: "+ size
  )
]

#for size in ("1", "2", "4", "8", "16", "32", "64", "128", "256", "512", "1024", "2048") [
  #figure(
  image("../results/linux/memory/sporadic_size_"+size+"_memory.png"),
    caption: "Sporadic benchmark memory usage for size: " + size
  )

  #figure(
    image("../results/linux/time/sporadic_size_"+size+".0.png"),
    caption: "Sporadic benchmark for size: "+ size
  )
]

#figure(
  image("../results/linux/memory/varying_size_0_memory.png"),
  caption: "Varying memory usage"
)

#figure(
  image("../results/linux/time/varying_size_varying.png"),
  caption: "Varying time usage"
)

#figure(
  image("../results/linux/genetic/genetic_benchmark_mean_time.png"),
  caption: "Genetic program mean time"
)

#for size in ("1", "2", "4", "8", "16", "32", "64", "128", "256", "512", "1024", "2048") [
  #figure(
    image("../results/macos/time/basic_size_"+size+".0.png"),
    caption: "Basic benchmark for size: "+ size
  )
]

#for size in ("1", "2", "4", "8", "16", "32", "64", "128", "256", "512", "1024", "2048") [
  #figure(
    image("../results/macos/time/sporadic_size_"+size+".0.png"),
    caption: "Sporadic benchmark for size: "+ size
  )
]

#figure(
  image("../results/macos/time/varying_size_varying.png"),
  caption: "Varying time usage"
)

#figure(
  image("../results/macos/genetic/genetic_benchmark_mean_time.png"),
  caption: "Genetic program mean time"
)

