USE [dnworld]
GO
/****** Object:  Table [dbo].[JobChangeLogs]    Script Date: 11/14/2026 8:54:16 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[JobChangeLogs](
	[JobChangeLogID] [int] IDENTITY(1,1) NOT NULL,
	[CharacterID] [bigint] NOT NULL,
	[JobCode] [tinyint] NOT NULL,
	[LogDate] [datetime2](0) NOT NULL,
 CONSTRAINT [PK_JobChangeLogs] PRIMARY KEY CLUSTERED 
(
	[JobChangeLogID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
) ON [DataFG1]
GO
/****** Object:  Index [IX_NNI_JobChangeLogs_CharacterID]    Script Date: 11/14/2026 8:54:37 AM ******/
CREATE NONCLUSTERED INDEX [IX_NNI_JobChangeLogs_CharacterID] ON [dbo].[JobChangeLogs]
(
	[CharacterID] ASC
)
INCLUDE([JobCode]) WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [IndexFG1]
GO
