USE [dnworld]
GO
/****** Object:  Table [dbo].[JobManagerLogs]    Script Date: 11/14/2026 8:54:16 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[JobManagerLogs](
	[LogDate] [datetime2](0) NULL,
	[ExcuteJobName] [nvarchar](100) COLLATE Chinese_PRC_CI_AS NULL,
	[IsSuccess] [bit] NULL
) ON [DataFG1]
GO
