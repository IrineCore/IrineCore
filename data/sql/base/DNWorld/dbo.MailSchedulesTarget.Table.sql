USE [dnworld]
GO
/****** Object:  Table [dbo].[MailSchedulesTarget]    Script Date: 11/14/2026 8:54:16 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[MailSchedulesTarget](
	[MailScheduleID] [int] NOT NULL,
	[CharacterID] [bigint] NOT NULL,
	[DeleteFlag] [bit] NOT NULL,
 CONSTRAINT [PK_MailSchedulesTarget] PRIMARY KEY CLUSTERED 
(
	[MailScheduleID] ASC,
	[CharacterID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
) ON [DataFG1]
GO
/****** Object:  Index [IX_NNF_MailSchedulesTarget_CharacterID]    Script Date: 11/14/2026 8:54:37 AM ******/
CREATE NONCLUSTERED INDEX [IX_NNF_MailSchedulesTarget_CharacterID] ON [dbo].[MailSchedulesTarget]
(
	[CharacterID] ASC
)
WHERE ([DeleteFlag]=(0))
WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [IndexFG1]
GO
ALTER TABLE [dbo].[MailSchedulesTarget] ADD  CONSTRAINT [DF_MailSchedulesTarget_DeleteFlag]  DEFAULT ((0)) FOR [DeleteFlag]
GO
