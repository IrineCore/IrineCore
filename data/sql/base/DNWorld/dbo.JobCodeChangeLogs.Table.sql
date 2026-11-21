USE [dnworld]
GO
/****** Object:  Table [dbo].[JobCodeChangeLogs]    Script Date: 11/14/2026 8:54:16 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[JobCodeChangeLogs](
	[JobCodeChangeLogID] [int] IDENTITY(1,1) NOT NULL,
	[CharacterID] [bigint] NOT NULL,
	[JobCodeBefore] [tinyint] NOT NULL,
	[JobCodeAfter] [tinyint] NOT NULL,
	[JobCodeChangeCode] [tinyint] NOT NULL,
	[ChangeDate] [datetime] NOT NULL
) ON [IndexFG1]
GO
/****** Object:  Index [IX_NC_JobCodeChangeLogs_CharacterID]    Script Date: 11/14/2026 8:54:16 AM ******/
CREATE CLUSTERED INDEX [IX_NC_JobCodeChangeLogs_CharacterID] ON [dbo].[JobCodeChangeLogs]
(
	[CharacterID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [IndexFG1]
GO
/****** Object:  Index [PK_JobCodeChangeLogs]    Script Date: 11/14/2026 8:54:37 AM ******/
ALTER TABLE [dbo].[JobCodeChangeLogs] ADD  CONSTRAINT [PK_JobCodeChangeLogs] PRIMARY KEY NONCLUSTERED 
(
	[JobCodeChangeLogID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, IGNORE_DUP_KEY = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [IndexFG1]
GO
