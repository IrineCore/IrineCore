USE [dnstaging]
GO
/****** Object:  Table [dbo].[CharacterActionLogs]    Script Date: 11/14/2026 9:00:09 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[CharacterActionLogs](
	[CharacterID] [bigint] NOT NULL,
	[LogDate] [datetime2](0) SPARSE  NULL,
	[LogFDate] [datetime2](0) NULL,
	[CharacterActionLogCode] [tinyint] NOT NULL,
	[Amount1] [bigint] NULL,
	[Amount2] [bigint] NULL,
	[Amount3] [bigint] SPARSE  NULL,
	[MapID] [int] NULL,
	[IP] [int] NULL,
	[LogFKey1] [bigint] NULL,
	[LogFKey2] [bigint] NULL,
	[LogFKey3] [bigint] SPARSE  NULL,
	[LogMessage1] [nvarchar](100) COLLATE Chinese_PRC_CI_AS SPARSE  NULL
) ON [PRIMARY]
GO
