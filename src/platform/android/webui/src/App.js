import {
  useEffect,
  useState
} from 'react';

import {
  AppBar,
  Box,
  Button,
  Link,
  TextField,
  Toolbar,
  Typography,
} from '@mui/material';

import {
  DataGrid,
  GridToolbarContainer,
  GridToolbarFilterButton,
  GridToolbarDensitySelector,
} from '@mui/x-data-grid';

import {
  Download as DownloadIcon
} from '@mui/icons-material';

import {
  useTheme
} from "@mui/material/styles";

const columns = [{
  field: 'fileName',
  headerName: 'Name',
  flex: 1,
}, {
  field: 'size',
  headerName: 'Size',
  type: 'number',
}, {
  field: 'date',
  headerName: 'Modified',
  type: 'number',
}, {
  field: 'actions',
  type: 'actions',
  headerName: 'Actions',
  cellClassName: 'actions',
  getActions: ({ id }) => {
    return [
      <Button
        variant="contained"
        size="small"
        style={{marginLeft: 16}}
        tabIndex={-1}>
        Run
      </Button>
    ];
  },
}];

function GridToolbarDownload(props) {
  let color = useTheme().palette.primary.main;
  let args = "";
  let join = "";
  console.log(props.rows);
  props.selections.forEach(i => {
    args += join + "f=" + encodeURIComponent(props.rows[i].fileName);
    join = "&";
  });

  return !props.selections.length ? null : (
    <a download="download.zip" href={`./api/download?${args}`} style={{color: color, textDecoration: 'none'}}>
      <Box sx={{display: 'flex', marginTop: '1px', alignItems: 'center'}} >
        <DownloadIcon />
        <Typography variant="caption">
          DOWNLOAD
        </Typography>
      </Box>
    </a>
  );
}

function AppToolbar(props) {
  return (
    <GridToolbarContainer>
      <GridToolbarFilterButton />
      <GridToolbarDensitySelector />
      <GridToolbarDownload {...props}/>
    </GridToolbarContainer>
  );
}

function FileList(props) {
  const [selectionModel, setSelectionModel] = useState([]);
  return (
    <Box sx={{flexGrow: 1}}>
      <AppBar position="static">
        <Toolbar>
          <Typography variant="h6" component="div" sx={{flexGrow: 1}}>
            SmallBASIC
          </Typography>
          <Box>
            <Link target="new" href="https://smallbasic.github.io" color="inherit">
              <Typography variant="h6" component="div" sx={{flexGrow: 1}}>
                https://smallbasic.github.io
              </Typography>
            </Link>
          </Box>
        </Toolbar>
      </AppBar>
      <Box sx={{height: 'calc(100vh - 5.5em)', width: '100%'}}>
        <DataGrid rows={props.rows}
                  columns={columns}
                  pageSize={5}
                  components={{Toolbar: AppToolbar}}
                  componentsProps={{toolbar: {selections: selectionModel, rows: props.rows}}}
                  onSelectionModelChange={(model) => setSelectionModel(model)}
                  selectionModel={selectionModel}
                  rowsPerPageOptions={[5]}
                  checkboxSelection
                  disableSelectionOnClick/>
      </Box>
    </Box>

  );
}

export default function App() {
  const [token, setToken] = useState("token=ABC123");
  const [rows, setRows] = useState([]);

  useEffect(() => {
    const getFiles = async () => {
      let response = await fetch('/api/files', {
        method: 'POST',
        headers: {
          'Content-Type': 'application/text;charset=utf-8'
        },
        body: token
      });
      setRows(await response.json());
    };
    getFiles().catch(console.error);
  }, [token, setRows]);

  return (
    <FileList rows={rows}/>
  );
}
