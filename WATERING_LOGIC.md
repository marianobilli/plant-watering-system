# Watering Logic - Detailed Explanation

## Overview

The system uses a **two-phase cycle** to water plants intelligently while allowing natural soil drying:

1. **Watering Phase** - Waters until upper limit reached
2. **Dry Cycle Phase** - Waits until lower limit reached before watering again

---

## Default Configuration (Per Plant Profile)

| Parameter | Default Value | Range | Description |
|-----------|--------------|-------|-------------|
| **Upper Limit** (Target) | 80% | 0-100% | Stop watering when this moisture level is reached |
| **Lower Limit** (Minimum) | 10% | 0-100% | Start watering when moisture drops to this level |
| **Water Amount** | 50 mL | 1-999 mL | Amount to water per iteration |
| **Soak Time** | 5 minutes | 1-30 min | Wait time for capillarity after each watering |
| **Check Interval** | 6 hours | 1-24 hours | How often to check moisture during dry cycle |

**Note:** Dry cycle is always enabled - it's core to the watering logic and essential for plant health.

---

## Phase 1: Watering Phase

**Trigger:** Moisture drops to or below **Lower Limit** (10%)

**Process:**
1. **Water** the configured amount (50 mL default)
2. **Wait** for soak time (5 minutes default) to allow capillarity
3. **Re-measure** soil moisture
4. **Check** if moisture is below upper limit (80%)
   - If **YES** → Repeat steps 1-4
   - If **NO** (reached 80% or higher) → Enter Dry Cycle Phase

**Example:**
```
Initial moisture: 8% (below 10% minimum)
→ Water 50mL
→ Wait 5 minutes (capillarity)
→ Measure: 45%
→ Still below 80%, so continue...
→ Water 50mL
→ Wait 5 minutes
→ Measure: 82%
→ Reached target! Enter dry cycle
```

---

## Phase 2: Dry Cycle Phase

**Trigger:** Moisture reaches or exceeds **Upper Limit** (80%)

**Process:**
1. **Wait** for check interval (6 hours default)
2. **Measure** soil moisture
3. **Check** if moisture is at or below lower limit (10%)
   - If **NO** (still above 10%) → Continue waiting (repeat steps 1-3)
   - If **YES** (dropped to 10% or below) → Enter Watering Phase

**Purpose:**
- Prevents constant watering
- Allows natural soil drying cycle
- Healthier for plant roots (prevents root rot)
- Mimics natural rainfall patterns

**Example:**
```
After watering: 82%
→ Wait 6 hours
→ Measure: 65%
→ Still above 10%, stay in dry cycle
→ Wait 6 hours
→ Measure: 42%
→ Still above 10%, stay in dry cycle
→ Wait 6 hours
→ Measure: 9%
→ Below 10%! Start watering phase
```

---

## Failsafe Protection

The system includes safety checks to prevent damage:

### 1. Moisture Not Increasing
**Problem:** Sensor or pump failure
**Detection:** After watering, moisture didn't increase from previous reading
**Action:**
- Stop watering immediately
- Display error: "Water not absorbing - check sensor/pump"
- User must resolve issue before system continues

### 2. Maximum Iterations
**Problem:** System can't reach target (leak, wrong calibration, etc.)
**Detection:** Watered 10 times without reaching upper limit
**Action:**
- Stop watering to prevent flooding
- Log warning for user review

---

## Why Soak Time is Critical

**Without soak time:**
```
Water 50mL → Measure immediately → Sensor reads 15% (water hasn't spread)
→ System thinks soil is still dry
→ Water another 50mL → Measure → Still reads low
→ Keeps watering → FLOOD!
```

**With soak time (5 minutes):**
```
Water 50mL → Wait 5 minutes → Water spreads via capillary action
→ Measure → Sensor reads 45% (accurate)
→ Water 50mL → Wait 5 minutes
→ Measure → 82% (reached target, stop)
```

**Capillary action timing:**
- **Sandy soil** (fast drainage): 3-5 minutes
- **Loam** (medium drainage): 5-10 minutes
- **Clay** (slow drainage): 10-15 minutes
- **Default (potting mix)**: 5 minutes

---

## Per-Plant Profiles (Phase 2 - Future)

In the final 6-plant system, each plant will have independent configuration:

```
Plant 1 (Cactus):
  - Upper: 40%, Lower: 5%
  - Amount: 20mL, Soak: 3min
  - Check: 12 hours

Plant 2 (Fern):
  - Upper: 90%, Lower: 60%
  - Amount: 100mL, Soak: 7min
  - Check: 4 hours

Plant 3 (Tomato):
  - Upper: 80%, Lower: 20%
  - Amount: 150mL, Soak: 10min
  - Check: 6 hours
```

Each plant operates independently with its own cycle.

---

## Why Dry Cycle is Always Enabled

The dry cycle is **always ON** in this system - it's not optional. Here's why:

```
Watering phase: Water when moisture ≤ 10% until 80%
Dry cycle phase: Wait (no watering) until moisture ≤ 10%
Check interval: 6 hours
```

**Benefits:**
- **Prevents root rot:** Constant moisture kills roots through oxygen deprivation
- **Mimics nature:** Natural rainfall has wet/dry cycles
- **Stronger roots:** Dry periods encourage deeper root growth
- **Prevents fungal growth:** Wet soil breeds mold and fungus
- **Better oxygen exchange:** Dry periods allow soil to breathe

**Without dry cycle:**
- Soil stays constantly wet (60-80% moisture)
- High risk of root rot and plant death
- Fungal infections and pest problems
- Weak, shallow root systems

The configurable parameters (10% minimum, 80% maximum) allow customization for different plant species while maintaining the healthy wet/dry cycle.

---

## Configuration via LCD Menu (POC)

Navigate to: **Main Menu → Settings**

```
Settings Menu:
├── Target (Upper limit)     → 80% (default)
├── Min (Lower limit)        → 10% (default)
├── Amount (per watering)    → 50mL (default)
├── Check Interval           → 6h (default)
└── Soak Time                → 5min (default)
```

**Note:** Dry cycle is always ON and not shown in menu - it's a core part of the system.

**Adjustment:**
- Use **UP/DOWN** buttons to change value
- Press **SELECT** to save
- Press **BACK** to cancel

---

## Real-World Example Timeline

**Plant:** Tomato in 5L pot with potting mix
**Configuration:** Target 80%, Min 10%, 50mL per watering, 5min soak, 6h check

**Day 1:**
```
00:00 - Moisture: 8% → START WATERING PHASE
00:00 - Water 50mL
00:05 - Measure: 42%
00:05 - Water 50mL (still below 80%)
00:10 - Measure: 81% → ENTER DRY CYCLE
06:10 - Check: 72% (above 10%, continue dry cycle)
12:10 - Check: 61%
18:10 - Check: 48%
```

**Day 2:**
```
00:10 - Check: 35%
06:10 - Check: 22%
12:10 - Check: 9% → START WATERING PHASE
12:10 - Water 50mL
12:15 - Measure: 45%
12:15 - Water 50mL
12:20 - Measure: 83% → ENTER DRY CYCLE
```

---

## Summary

✅ **Watering Phase:** Water in 50mL increments with 5min soak until 80% reached
✅ **Dry Cycle Phase:** Check every 6 hours, water only when ≤10%
✅ **Failsafe:** Stop if moisture doesn't increase
✅ **Configurable:** All parameters adjustable per plant
✅ **Intelligent:** Mimics natural watering patterns for healthier plants
