USE [dnworld]
GO
/****** Object:  Table [dbo].[ErrorLogs]    Script Date: 11/14/2026 8:54:15 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[ErrorLogs](
	[errorLogSN] [bigint] IDENTITY(-1,-1) NOT NULL,
	[loginName] [nvarchar](128) COLLATE Chinese_PRC_CI_AS NOT NULL,
	[hostName] [nvarchar](128) COLLATE Chinese_PRC_CI_AS NOT NULL,
	[errorNumber] [int] NOT NULL,
	[errorSeverity] [int] NULL,
	[errorState] [int] NULL,
	[errorProcedure] [nvarchar](128) COLLATE Chinese_PRC_CI_AS NULL,
	[errorLine] [int] NULL,
	[errorMessage] [nvarchar](max) COLLATE Chinese_PRC_CI_AS NOT NULL,
	[occurredDate] [datetime] NOT NULL,
 CONSTRAINT [PK_ErrorLogs] PRIMARY KEY CLUSTERED 
(
	[errorLogSN] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
) ON [DataFG1] TEXTIMAGE_ON [DataFG1]
GO
