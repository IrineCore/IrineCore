USE [dnstaging]
GO
/****** Object:  Table [dbo].[PetSkillLogs]    Script Date: 11/14/2026 9:00:09 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[PetSkillLogs](
	[BasisDate] [datetime2](0) NOT NULL,
	[PetItemSerial] [bigint] NOT NULL,
	[SkillID] [int] NOT NULL,
	[WorldID] [int] NOT NULL
) ON [PRIMARY]
GO
