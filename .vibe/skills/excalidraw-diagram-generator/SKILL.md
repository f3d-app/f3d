---
name: excalidraw-diagram-generator
description: 'Generate Excalidraw diagrams from natural language descriptions. Use when asked to "create a diagram", "make a flowchart", "visualize a process", "draw a system architecture", "create a mind map", or "generate an Excalidraw file". Supports flowcharts, relationship diagrams, mind maps, and system architecture diagrams. Outputs .excalidraw JSON files that can be opened directly in Excalidraw.'
---

# Excalidraw Diagram Generator

A skill for generating Excalidraw-format diagrams from natural language descriptions. This skill helps create visual representations of processes, systems, relationships, and ideas without manual drawing.

## When to Use This Skill

Use this skill when users request:

- "Create a diagram showing..."
- "Make a flowchart for..."
- "Visualize the process of..."
- "Draw the system architecture of..."
- "Generate a mind map about..."
- "Create an Excalidraw file for..."
- "Show the relationship between..."
- "Diagram the workflow of..."

**Supported diagram types:**
- 📊 **Flowcharts**: Sequential processes, workflows, decision trees
- 🔗 **Relationship Diagrams**: Entity relationships, system components, dependencies
- 🧠 **Mind Maps**: Concept hierarchies, brainstorming results, topic organization
- 🏗️ **Architecture Diagrams**: System design, module interactions, data flow
- 📈 **Data Flow Diagrams (DFD)**: Data flow visualization, data transformation processes
- 🏊 **Business Flow (Swimlane)**: Cross-functional workflows, actor-based process flows
- 📦 **Class Diagrams**: Object-oriented design, class structures and relationships
- 🔄 **Sequence Diagrams**: Object interactions over time, message flows
- 🗃️ **ER Diagrams**: Database entity relationships, data models

## Prerequisites

- Clear description of what should be visualized
- Identification of key entities, steps, or concepts
- Understanding of relationships or flow between elements

## Step-by-Step Workflow

### Step 1: Understand the Request

Analyze the user's description to determine:
1. **Diagram type** (flowchart, relationship, mind map, architecture)
2. **Key elements** (entities, steps, concepts)
3. **Relationships** (flow, connections, hierarchy)
4. **Complexity** (number of elements)

### Step 2: Choose the Appropriate Diagram Type

| User Intent | Diagram Type | Example Keywords |
|-------------|--------------|------------------|
| Process flow, steps, procedures | **Flowchart** | "workflow", "process", "steps", "procedure" |
| Connections, dependencies, associations | **Relationship Diagram** | "relationship", "connections", "dependencies", "structure" |
| Concept hierarchy, brainstorming | **Mind Map** | "mind map", "concepts", "ideas", "breakdown" |
| System design, components | **Architecture Diagram** | "architecture", "system", "components", "modules" |
| Data flow, transformation processes | **Data Flow Diagram (DFD)** | "data flow", "data processing", "data transformation" |
| Cross-functional processes, actor responsibilities | **Business Flow (Swimlane)** | "business process", "swimlane", "actors", "responsibilities" |
| Object-oriented design, class structures | **Class Diagram** | "class", "inheritance", "OOP", "object model" |
| Interaction sequences, message flows | **Sequence Diagram** | "sequence", "interaction", "messages", "timeline" |
| Database design, entity relationships | **ER Diagram** | "database", "entity", "relationship", "data model" |

### Step 3: Extract Structured Information

**For Flowcharts:**
- List of sequential steps
- Decision points (if any)
- Start and end points

**For Relationship Diagrams:**
- Entities/nodes (name + optional description)
- Relationships between entities (from → to, with label)

**For Mind Maps:**
- Central topic
- Main branches (3-6 recommended)
- Sub-topics for each branch (optional)

**For Data Flow Diagrams (DFD):**
- Data sources and destinations (external entities)
- Processes (data transformations)
- Data stores (databases, files)
- Data flows (arrows showing data movement from left-to-right or from top-left to bottom-right)
- **Important**: Do not represent process order, only data flow

**For Business Flow (Swimlane):**
- Actors/roles (departments, systems, people) - displayed as header columns
- Process lanes (vertical lanes under each actor)
- Process boxes (activities within each lane)
- Flow arrows (connecting process boxes, including cross-lane handoffs)

**For Class Diagrams:**
- Classes with names
- Attributes with visibility (+, -, #)
- Methods with visibility and parameters
- Relationships: inheritance (solid line + white triangle), implementation (dashed line + white triangle), association (solid line), dependency (dashed line), aggregation (solid line + white diamond), composition (solid line + filled diamond)
- Multiplicity notations (1, 0..1, 1..*, *)

**For Sequence Diagrams:**
- Objects/actors (arranged horizontally at top)
- Lifelines (vertical lines from each object)
- Messages (horizontal arrows between lifelines)
- Synchronous messages (solid arrow), asynchronous messages (dashed arrow)
- Return values (dashed arrows)
- Activation boxes (rectangles on lifelines during execution)
- Time flows from top to bottom

**For ER Diagrams:**
- Entities (rectangles with entity names)
- Attributes (listed inside entities)
- Primary keys (underlined or marked with PK)
- Foreign keys (marked with FK)
- Relationships (lines connecting entities)
- Cardinality: 1:1 (one-to-one), 1:N (one-to-many), N:M (many-to-many)
- Junction/associative entities for many-to-many relationships (dashed rectangles)

### Step 4: Generate the Excalidraw JSON

Create the `.excalidraw` file with appropriate elements:

**Available element types:**
- `rectangle`: Boxes for entities, steps, concepts
- `ellipse`: Alternative shapes for emphasis
- `diamond`: Decision points
- `arrow`: Directional connections
- `text`: Labels and annotations

**Key properties to set:**
- **Position**: `x`, `y` coordinates
- **Size**: `width`, `height`
- **Style**: `strokeColor`, `backgroundColor`, `fillStyle`
- **Font**: `fontFamily: 5` (Excalifont - **required for all text elements**)
- **Text**: Embedded text for labels
- **Connections**: `points` array for arrows

**Important**: All text elements must use `fontFamily: 5` (Excalifont) for consistent visual appearance.

### Step 5: Format the Output

Structure the complete Excalidraw file:

```json
{
  "type": "excalidraw",
  "version": 2,
  "source": "https://excalidraw.com",
  "elements": [
    // Array of diagram elements
  ],
  "appState": {
    "viewBackgroundColor": "#ffffff",
    "gridSize": 20
  },
  "files": {}
}
```

### Step 6: Save and Provide Instructions

1. Save as `<descriptive-name>.excalidraw`
2. Inform user how to open:
   - Visit https://excalidraw.com
   - Click "Open" or drag-and-drop the file
   - Or use Excalidraw VS Code extension

## Best Practices

### Element Count Guidelines

| Diagram Type | Recommended Count | Maximum |
|--------------|-------------------|---------|
| Flowchart steps | 3-10 | 15 |
| Relationship entities | 3-8 | 12 |
| Mind map branches | 4-6 | 8 |
| Mind map sub-topics per branch | 2-4 | 6 |

### Layout Tips

1. **Start positions**: Center important elements, use consistent spacing
2. **Spacing**: 
   - Horizontal gap: 200-300px between elements
   - Vertical gap: 100-150px between rows
3. **Colors**: Use consistent color scheme
   - Primary elements: Light blue (`#a5d8ff`)
   - Secondary elements: Light green (`#b2f2bb`)
   - Important/Central: Yellow (`#ffd43b`)
   - Alerts/Warnings: Light red (`#ffc9c9`)
4. **Text sizing**: 16-24px for readability
5. **Font**: Always use `fontFamily: 5` (Excalifont) for all text elements
6. **Arrow style**: Use straight arrows for simple flows, curved for complex relationships

### Complexity Management

**If user request has too many elements:**
- Suggest breaking into multiple diagrams
- Focus on main elements first
- Offer to create detailed sub-diagrams

## Example Prompts and Responses

### Example 1: Simple Flowchart

**User:** "Create a flowchart for user registration"

**Agent generates:**
1. Extract steps: "Enter email" → "Verify email" → "Set password" → "Complete"
2. Create flowchart with 4 rectangles + 3 arrows
3. Save as `user-registration-flow.excalidraw`

### Example 2: Relationship Diagram

**User:** "Diagram the relationship between User, Post, and Comment entities"

**Agent generates:**
1. Entities: User, Post, Comment
2. Relationships: User → Post ("creates"), User → Comment ("writes"), Post → Comment ("contains")
3. Save as `user-content-relationships.excalidraw`

### Example 3: Mind Map

**User:** "Mind map about machine learning concepts"

**Agent generates:**
1. Center: "Machine Learning"
2. Branches: Supervised Learning, Unsupervised Learning, Reinforcement Learning, Deep Learning
3. Sub-topics under each branch
4. Save as `machine-learning-mindmap.excalidraw`

## Troubleshooting

| Issue | Solution |
|-------|----------|
| Elements overlap | Increase spacing between coordinates |
| Text doesn't fit in boxes | Increase box width or reduce font size |
| Too many elements | Break into multiple diagrams |
| Unclear layout | Use grid layout (rows/columns) or radial layout (mind maps) |
| Colors inconsistent | Define color palette upfront based on element types |

## Advanced Techniques

### Grid Layout (for Relationship Diagrams)
```javascript
const columns = Math.ceil(Math.sqrt(entityCount));
const x = startX + (index % columns) * horizontalGap;
const y = startY + Math.floor(index / columns) * verticalGap;
```

### Radial Layout (for Mind Maps)
```javascript
const angle = (2 * Math.PI * index) / branchCount;
const x = centerX + radius * Math.cos(angle);
const y = centerY + radius * Math.sin(angle);
```

### Auto-generated IDs
Use timestamp + random string for unique IDs:
```javascript
const id = Date.now().toString(36) + Math.random().toString(36).substr(2);
```

## Output Format

Always provide:
1. ✅ Complete `.excalidraw` JSON file
2. 📊 Summary of what was created
3. 📝 Element count
4. 💡 Instructions for opening/editing

## Validation Checklist

Before delivering the diagram:
- [ ] All elements have unique IDs
- [ ] Coordinates prevent overlapping
- [ ] Text is readable (font size 16+)
- [ ] **All text elements use `fontFamily: 5` (Excalifont)**
- [ ] Arrows connect logically
- [ ] Colors follow consistent scheme
- [ ] File is valid JSON
- [ ] Element count is reasonable (<20 for clarity)

## Icon Libraries (Optional Enhancement)

For specialized diagrams (e.g., AWS/GCP/Azure architecture diagrams), you can use pre-made icon libraries from Excalidraw.

### When User Requests Icons

**If user asks for AWS/cloud architecture diagrams or mentions wanting to use specific icons:**

1. **Check if library exists**: Look for `libraries/<library-name>/reference.md`
2. **If library exists**: Proceed to use icons (see AI Assistant Workflow below)
3. **If library does NOT exist**: Respond with setup instructions:

   ```
   To use [AWS/GCP/Azure/etc.] architecture icons, please follow these steps:
   
   1. Visit https://libraries.excalidraw.com/
   2. Search for "[AWS Architecture Icons/etc.]" and download the .excalidrawlib file
   3. Create directory: skills/excalidraw-diagram-generator/libraries/[icon-set-name]/
   4. Place the downloaded file in that directory
   5. Run the splitter script:
      python skills/excalidraw-diagram-generator/scripts/split-excalidraw-library.py skills/excalidraw-diagram-generator/libraries/[icon-set-name]/
   
   After setup is complete, I can create your diagram using the actual icons.
   
   Alternatively, I can create the diagram now using simple shapes (rectangles, ellipses) 
   which you can later replace with icons manually in Excalidraw.
   ```

### AI Assistant Workflow

**When icon libraries are available in `libraries/`:**

**RECOMMENDED APPROACH: Use Python Scripts (Efficient & Reliable)**

1. **Create base diagram structure** — Create `.excalidraw` file with basic layout
2. **Add icons using Python script**:
   ```bash
   python skills/excalidraw-diagram-generator/scripts/add-icon-to-diagram.py \
     <diagram-path> <icon-name> <x> <y> [--label "Text"] [--library-path PATH]
   ```
3. **Add connecting arrows**:
   ```bash
   python skills/excalidraw-diagram-generator/scripts/add-arrow.py \
     <diagram-path> <from-x> <from-y> <to-x> <to-y> [--label "Text"] [--style solid|dashed|dotted]
   ```

**Benefits of Python Script Approach**:
- ✅ **No token consumption**: Icon JSON data never enters AI context
- ✅ **Accurate transformations**: Coordinate calculations handled deterministically
- ✅ **ID management**: Automatic UUID generation prevents conflicts
- ✅ **Reliable**: No risk of coordinate miscalculation or ID collision

### Fallback: No Icons Available

**If no icon libraries are set up:**
- Create diagrams using basic shapes (rectangles, ellipses, arrows)
- Use color coding and text labels to distinguish components
- Inform user they can add icons later or set up libraries for future diagrams

## References

See bundled references for:
- `references/excalidraw-schema.md` - Complete Excalidraw JSON schema
- `references/element-types.md` - Detailed element type specifications
- `scripts/split-excalidraw-library.py` - Tool to split `.excalidrawlib` files
- `scripts/add-icon-to-diagram.py` - Add icons to existing diagrams
- `scripts/add-arrow.py` - Add arrows to existing diagrams
- `scripts/README.md` - Documentation for library tools
- `scripts/.gitignore` - Prevents local Python artifacts from being committed

## Limitations

- Complex curves are simplified to straight/basic curved lines
- Hand-drawn roughness is set to default (1)
- No embedded images support in auto-generation
- Maximum recommended elements: 20 per diagram
- No automatic collision detection (use spacing guidelines)

## Future Enhancements

Potential improvements:
- Auto-layout optimization algorithms
- Import from Mermaid/PlantUML syntax
- Template library expansion
- Interactive editing after generation
