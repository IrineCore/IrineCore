USE [dnstaging]
GO
/****** Object:  Table [dbo].[PetSkillLogs_20131003]    Script Date: 11/14/2026 9:00:08 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[PetSkillLogs_20131003](
	[BasisDate] [datetime2](0) NOT NULL,
	[PetItemSerial] [bigint] NOT NULL,
	[SkillID] [int] NOT NULL,
	[WorldID] [int] NOT NULL
) ON [PRIMARY]
GO
/****** Object:  Index [IX_NC_PetSkillLogs_20131003_BasisDate]    Script Date: 11/14/2026 9:00:08 AM ******/
CREATE CLUSTERED INDEX [IX_NC_PetSkillLogs_20131003_BasisDate] ON [dbo].[PetSkillLogs_20131003]
(
	[BasisDate] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
ALTER TABLE [dbo].[PetSkillLogs_20131003]  WITH CHECK ADD  CONSTRAINT [CK_PetSkillLogs_20131003_BasisDate] CHECK  (([BasisDate]>='2013-10-03' AND [BasisDate]<'2013-10-04'))
GO
ALTER TABLE [dbo].[PetSkillLogs_20131003] CHECK CONSTRAINT [CK_PetSkillLogs_20131003_BasisDate]
GO
