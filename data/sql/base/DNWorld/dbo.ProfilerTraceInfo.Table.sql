USE [dnworld]
GO
/****** Object:  Table [dbo].[ProfilerTraceInfo]    Script Date: 11/14/2026 8:54:16 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[ProfilerTraceInfo](
	[TraceID] [int] NULL,
	[TraceFile] [nvarchar](245) COLLATE Chinese_PRC_CI_AS NULL
) ON [DataFG1]
GO
