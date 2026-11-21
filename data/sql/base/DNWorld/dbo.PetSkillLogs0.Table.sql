USE [dnworld]
GO
/****** Object:  Table [dbo].[PetSkillLogs0]    Script Date: 11/14/2026 8:54:16 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[PetSkillLogs0](
	[PetItemSerial] [bigint] NOT NULL,
	[SkillID] [int] NOT NULL,
	[BasisDate] [datetime2](0) NOT NULL
) ON [PRIMARY]
GO
/****** Object:  Index [IX_NC_PetSkillLogs0_BasisDate]    Script Date: 11/14/2026 8:54:16 AM ******/
CREATE CLUSTERED INDEX [IX_NC_PetSkillLogs0_BasisDate] ON [dbo].[PetSkillLogs0]
(
	[BasisDate] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
