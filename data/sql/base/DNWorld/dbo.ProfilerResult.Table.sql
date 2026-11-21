USE [dnworld]
GO
/****** Object:  Table [dbo].[ProfilerResult]    Script Date: 11/14/2026 8:54:16 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[ProfilerResult](
	[StartTime] [datetime] NULL,
	[EndTime] [datetime] NULL,
	[TextData] [ntext] COLLATE Chinese_PRC_CI_AS NULL,
	[HostName] [nvarchar](256) COLLATE Chinese_PRC_CI_AS NULL,
	[LoginName] [nvarchar](256) COLLATE Chinese_PRC_CI_AS NULL,
	[Duration] [bigint] NULL,
	[Reads] [bigint] NULL,
	[Writes] [bigint] NULL,
	[CPU] [int] NULL,
	[ObjectID] [int] NULL,
	[ServerName] [nvarchar](256) COLLATE Chinese_PRC_CI_AS NULL,
	[EventClass] [int] NULL,
	[ObjectType] [int] NULL,
	[ObjectName] [nvarchar](256) COLLATE Chinese_PRC_CI_AS NULL,
	[DatabaseName] [nvarchar](256) COLLATE Chinese_PRC_CI_AS NULL
) ON [DataFG1] TEXTIMAGE_ON [DataFG1]
GO
