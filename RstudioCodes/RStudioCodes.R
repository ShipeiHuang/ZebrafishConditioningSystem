# 1) Read Files (you should replace the directory below with your directory)
top <- read.csv("C:/Users/User/Desktop/SampleFishUpperChamber.csv",
                header = TRUE, stringsAsFactors = FALSE)

bot <- read.csv("C:/Users/User/Desktop/SampleFishLowerChamber.csv",
                header = TRUE, stringsAsFactors = FALSE)

# 2) Position
top[[2]] <- suppressWarnings(as.numeric(top[[2]]))
bot[[2]] <- suppressWarnings(as.numeric(bot[[2]]))

# 3) Data Filtering
top <- top[!is.na(top[[2]]), ]
bot <- bot[!is.na(bot[[2]]), ]

# 4) Define x vector
xTop <- top[[2]]
xBot <- bot[[2]]

# Helper to compute mean of +1/-1 scoring based on sign of x
score_mean <- function(vec, greater_is_plus = TRUE) {
  if (greater_is_plus) {
    # x > 0 -> +1, x < 0 -> -1, x == 0 -> 0
    mean(ifelse(vec > 0,  1, ifelse(vec < 0, -1, 0)))
  } else {
    # x > 0 -> -1, x < 0 -> +1, x == 0 -> 0
    mean(ifelse(vec > 0, -1, ifelse(vec < 0,  1, 0)))
  }
}

# 5) Compute TX's PI
# Baseline = 240 seconds total: first 120 seconds before switch + second 90 seconds after switch
# T0/T1/T2/T3/T4/T5 = 180 seconds total respectively: first 90 seconds + second 90 seconds

# Top fish/chamber
# You should replace xTop [1:90] and XTop [91:180] to XTop[1:120] and XTop [121:240] for baseline period
PI1Top_T <- score_mean(xTop[1:90],    greater_is_plus = TRUE)
PI2Top_T <- score_mean(xTop[91:180],  greater_is_plus = FALSE)

PITop_T <- (PI1Top_T + PI2Top_T) / 2

# Bottom fish/chamber
# You should replace xBot [1:90] and XBot [91:180] to XBot[1:120] and XBot [121:240] for baseline period
PI1Bot_T <- score_mean(xBot[1:90],    greater_is_plus = TRUE)
PI2Bot_T <- score_mean(xBot[91:180],  greater_is_plus = FALSE)

PIBot_T <- (PI1Bot_T + PI2Bot_T) / 2

# 6) Print results
PITop_T

PIBot_T
