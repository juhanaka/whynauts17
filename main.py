import runner

def main():
  window_length = 10
  min_diff = 0.001
  tick_seconds = 0.2
  r = runner.Runner()
  r.run(window_length, min_diff, tick_seconds)

if __name__ == "__main__":
  main()
