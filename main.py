import runner

def main():
  window_length = 5
  min_diff = 0.001
  tick_seconds = 0.01
  r = runner.Runner()
  r.run(window_length, min_diff, tick_seconds)

if __name__ == "__main__":
  main()
