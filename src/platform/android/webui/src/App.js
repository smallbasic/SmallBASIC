import {
  Fragment,
  useState
} from 'react';

import {
  Alert,
  AppBar,
  Box,
  Button,
  CssBaseline,
  Link,
  Snackbar,
  TextField,
  Toolbar,
  Typography,
} from '@mui/material';

import {
  DataGrid,
  GridToolbarColumnsButton,
  GridToolbarContainer,
  GridToolbarDensitySelector,
  GridToolbarExport,
  GridToolbarFilterButton,
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
  valueGetter: (params) => {
    return params.row.fileName;
  },
  flex: 1,
}, {
  field: 'size',
  headerName: 'Size',
  type: 'number',
}, {
  field: 'date',
  headerName: 'Modified',
  type: 'date',
  minWidth: 110,
  renderCell: (params) => {
    return new Date(params.row.date).toLocaleDateString();
  }
}];

function getFetchHeader(body) {
  return {
    method: 'POST',
    headers: {'Content-Type': 'application/text;charset=utf-8'},
    body: body
  };
}

function callApi(api, body, success, fail) {
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

function getFiles(success, fail) {
  callApi('/api/files', "", success, fail);
}

function login(token, success, fail) {
  let body = "token=" + token;
  callApi('/api/login', body, success, fail);
}

function upload(name, data, success, fail) {
  let body = "fileName=" + encodeURIComponent(name) + "&data=" + data;
  callApi('/api/upload', body, success, fail);
}

function renameFile(from, to, success, fail) {
  let body = "from=" + encodeURIComponent(from) + "&to=" + encodeURIComponent(to);
  callApi('/api/rename', body, success, fail);
}

function copyFiles(event, success, progress, fail) {
  const fileReader = new FileReader();
  const input = event.target;
  const files = input.files;
  let index = 0;
  fileReader.onload = () => {
    upload(files[index].name, fileReader.result, () => {
      if (++index < files.length) {
        progress(index);
        fileReader.readAsDataURL(files[index]);
      } else {
        getFiles(success, fail);
        // reset input control
        input.value = input.defaultValue;
      }
    }, fail);
  };
  fileReader.readAsDataURL(files[index]);
}

function GridToolbarDownload(props) {
  let color = useTheme().palette.primary.main;
  let args = "";
  let join = "";
  let download = props.selections.length === 1 ? props.rows[props.selections[0]].fileName : "smallbasic-files.zip";

  props.selections.forEach(i => {
    args += join + "f=" + encodeURIComponent(props.rows[i].fileName);
    join = "&";
  });
  if (props.selections.length === props.rows.length) {
    args = "";
  }

  return !props.selections.length ? null : (
    <a download={download} href={`./api/download?${args}`} style={{color: color, textDecoration: 'none'}}>
      <Box sx={{display: 'flex', marginTop: '1px', alignItems: 'center'}} >
        <DownloadIcon />
        <Typography variant="caption">
          DOWNLOAD
        </Typography>
      </Box>
    </a>
  );
}

function ErrorMessage(props) {
  const handleClose = (event, reason) => {
    if (reason !== 'clickaway') {
      props.setError(null);
    }
  };

  return (
    <Snackbar open={props.error !== null} autoHideDuration={5000} onClose={handleClose}>
      <Alert onClose={handleClose} severity={props.severity} sx={{width: '100%'}}>
        {props.error}
      </Alert>
    </Snackbar>
  );
}

function GridToolbarUpload(props) {
  const [error, setError] = useState(null);
  const [severity, setSeverity] = useState("error");

  const handleUpload = (event) => {
    const length = event.target.files.length;
    setSeverity("error");
    copyFiles(event, (newRows) => {
      props.setRows(newRows);
      setSeverity("success");
      setError(`Uploaded ${length} files`);
    }, (count) => {
      setSeverity("success");
      setError(`Uploaded ${count} files`);
    }, (error) => {
      setSeverity("error");
      setError(error);
      // show any successful uploads
      getFiles((newRows) => {
        props.setRows(newRows);
      }, (error)=> setError(error));
    });
  };

  return (
    <Fragment>
      <ErrorMessage error={error} setError={setError} severity={severity}/>
      <Button color="primary" size="small" component="label" sx={{marginLeft: '-4px'}}>
        <input hidden multiple type="file" onChange={handleUpload}/>
        <UploadIcon />
        UPLOAD
      </Button>
    </Fragment>
  );
}

function AppToolbar(props) {
  return (
    <GridToolbarContainer>
      <GridToolbarFilterButton />
      <GridToolbarColumnsButton />
      <GridToolbarExport />
      <GridToolbarDensitySelector />
      <GridToolbarUpload {...props}/>
      <GridToolbarDownload {...props}/>
    </GridToolbarContainer>
  );
}

function onCellEditCommit(props, params, setError) {
  props.rows.forEach((row) => {
    if (row.id === params.id) {
      renameFile(row.fileName, params.value, () => {
        row.fileName = params.value;
        props.setRows(props.rows.slice());
      }, (error) => {
        props.setRows(props.rows.slice());
        setError(error);
      });
    }
  });
};

function FileList(props) {
  const [selectionModel, setSelectionModel] = useState([]);
  const [error, setError] = useState(null);

  const toolbarProps = {
    selections: selectionModel,
    setRows: props.setRows,
    rows: props.rows
  };

  const sorting = {
    sorting: {
      sortModel: [{ field: 'fileName', sort: 'asc' }],
    },
  };

  return (
    <Fragment>
      <ErrorMessage error={error} setError={setError} />
      <DataGrid rows={props.rows}
                initialState={sorting}
                columns={columns}
                onCellEditCommit={(params) => onCellEditCommit(props, params, setError)}
                autoPageSize
                components={{Toolbar: AppToolbar}}
                componentsProps={{toolbar: toolbarProps}}
                onSelectionModelChange={(model) => setSelectionModel(model)}
                selectionModel={selectionModel}
                rowsPerPageOptions={[5]}
                checkboxSelection
                disableSelectionOnClick/>
    </Fragment>
  );
}

function TokenInput(props) {
  const [token, setToken] = useState("");
  const [error, setError] = useState(false);

  const onClick = () => {
    login(token, (data) => {
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
                smallbasic.github.io
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
