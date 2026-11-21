USE [dnworld]
GO
/****** Object:  Table [dbo].[PupilCharacters]    Script Date: 11/14/2026 8:54:16 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[PupilCharacters](
	[PupilCharacterID] [bigint] NOT NULL,
	[NewTeacherBlockDate] [smalldatetime] NOT NULL,
 CONSTRAINT [PK_PupilCharacters] PRIMARY KEY CLUSTERED 
(
	[PupilCharacterID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
) ON [DataFG1]
GO
