import {
  useState
} from 'react';

import {
  AppBar,
  Box,
  Button,
  CssBaseline,
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
  Download as DownloadIcon,
  Upload as UploadIcon
} from '@mui/icons-material';

import {
  useTheme
} from "@mui/material/styles";

const columns = [{
  field: 'fileName',
  headerName: 'Name',
  editable: true,
  preProcessEditCellProps: (params) => {
    const hasError = !params.props.value.endsWith(".bas");
    return {...params.props, error: hasError};
  } ,
  flex: 1,
}, {
  field: 'size',
  headerName: 'Size',
  type: 'number',
}, {
  field: 'date',
  headerName: 'Modified',
  type: 'number',
}];

function getFetchHeader(body) {
  return {
    method: 'POST',
    headers: {'Content-Type': 'application/text;charset=utf-8'},
    body: body
  };
}

function fetchApi(api, body, success, fail) {
  fetch(api, getFetchHeader(body))
    .then(response => response.json())
    .then((response) => {
      if (response.error) {
        fail(response.error);
      } else {
        success(response);
      }
    })
    .catch(fail);
}

function getFiles(token, success, fail) {
  let body = token ? ("token=" + token) : "";
  fetchApi('/api/files', body, success, fail);
}

function upload(name, data, success, fail) {
  let body = "fileName=" + encodeURIComponent(name) + "&data=" + encodeURIComponent(data);
  fetchApi('/api/upload', body, success, fail);
}

function renameFile(from, to, success, fail) {
  let body = "from=" + encodeURIComponent(from) + "&to=" + encodeURIComponent(to);
  fetchApi('/api/rename', body, success, fail);
}

function copyFiles(event, success, fail) {
  const fileReader = new FileReader();
  const input = event.target;
  const files = input.files;
  let index = 0;
  fileReader.onload = () => {
    upload(files[index].name, fileReader.result, () => {
      if (++index < files.length) {
        fileReader.readAsText(files[index]);
      } else {
        getFiles(null, success, fail);
        // reset input control
        input.value = input.defaultValue;
      }
    }, fail);
  };
  fileReader.readAsText(files[index]);
}

function GridToolbarDownload(props) {
  let color = useTheme().palette.primary.main;
  let args = "";
  let join = "";

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

function GridToolbarUpload(props) {
  const handleUpload = (event) => {
    copyFiles(event, (newRows) => {
      props.setRows(newRows);
    }, (error) => {
      // show toast message
      console.log(error);
    });
  };

  return (
    <Button color="primary" size="small" component="label" sx={{marginLeft: '-4px'}}>
      <input accept=".bas" hidden multiple type="file" onChange={handleUpload}/>
      <UploadIcon />
      UPLOAD
    </Button>
  );
}

function AppToolbar(props) {
  return (
    <GridToolbarContainer>
      <GridToolbarFilterButton />
      <GridToolbarDensitySelector />
      <GridToolbarUpload {...props}/>
      <GridToolbarDownload {...props}/>
    </GridToolbarContainer>
  );
}

function onCellEditCommit(props, params) {
  props.rows.forEach((row) => {
    if (row.id === params.id) {
      renameFile(row.fileName, params.value, () => {
        getFiles(null, (data) => {
          props.setRows(data);
        }, (error) => {
          console.log(error);
        });
      }, (error) => {
        console.log(error);
      });
    }
  });
};

function FileList(props) {
  const [selectionModel, setSelectionModel] = useState([]);

  const toolbarProps = {
    selections: selectionModel,
    setRows: props.setRows,
    rows: props.rows
  };

  return (
    <DataGrid rows={props.rows}
              columns={columns}
              onCellEditCommit={(params) => onCellEditCommit(props, params)}
              pageSize={5}
              components={{Toolbar: AppToolbar}}
              componentsProps={{toolbar: toolbarProps}}
              onSelectionModelChange={(model) => setSelectionModel(model)}
              selectionModel={selectionModel}
              rowsPerPageOptions={[5]}
              checkboxSelection
              disableSelectionOnClick/>
  );
}

function TokenInput(props) {
  const [token, setToken] = useState("");
  const [error, setError] = useState(false);

  const onClick = () => {
    getFiles(token, (data) => {
      props.setRows(data);
      props.setToken(token);
    }, () => {
      setError(true);
    });
  };

  const onChange = (event) => {
    setToken(event.target.value);
    if (!event.target.value) {
      setError(false);
    }
  };

  const onKeyPress = (event) => {
    if (event.key === 'Enter') {
      onClick();
    }
  };

  const help = "Enter the access token displayed on the SmallBASIC [About] screen.";
  let helperText = error ? "Invalid token. " + help : help;

  return (
    <Box sx={{display: 'flex', justifyContent: 'center', alignItems: 'center', height: '100%'}}>
      <Box sx={{display: 'flex', flexDirection: 'column', marginBottom: '10em'}}>
        <Box sx={{marginBottom: '1em'}}>
          <TextField sx={{width: '20em'}}
                     error={error}
                     value={token}
                     onChange={onChange}
                     onKeyPress={onKeyPress}
                     helperText={helperText}
                     autoFocus
                     label="Enter your access token"/>
        </Box>
        <Box>
          <Button onClick={onClick} variant="contained">Submit</Button>
        </Box>
      </Box>
    </Box>
  );
}

export default function App() {
  const [token, setToken] = useState(null);
  const [rows, setRows] = useState([]);

  let content;
  if (token) {
    content = <FileList setRows={setRows} rows={rows} />;
  } else {
    content = <TokenInput setRows={setRows} setToken={setToken} token={token}/>;
  }

  return (
    <Box>
      <CssBaseline/>
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
        {content}
      </Box>
    </Box>
  );
}
