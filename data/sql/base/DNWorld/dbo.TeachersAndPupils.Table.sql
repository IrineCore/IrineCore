USE [dnworld]
GO
/****** Object:  Table [dbo].[TeachersAndPupils]    Script Date: 11/14/2026 8:54:15 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[TeachersAndPupils](
	[TeacherCharacterID] [bigint] NOT NULL,
	[PupilCharacterID] [bigint] NOT NULL,
	[FavorPoint] [int] NOT NULL,
	[GraduateFlag] [bit] NOT NULL,
	[RegisterDate] [datetime2](0) NOT NULL,
 CONSTRAINT [PK_TeachersAndPupils] PRIMARY KEY CLUSTERED 
(
	[TeacherCharacterID] ASC,
	[PupilCharacterID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
) ON [DataFG1]
GO
/****** Object:  Index [IX_NN_TeachersAndPupils_PupilCharacterID]    Script Date: 11/14/2026 8:54:37 AM ******/
CREATE NONCLUSTERED INDEX [IX_NN_TeachersAndPupils_PupilCharacterID] ON [dbo].[TeachersAndPupils]
(
	[PupilCharacterID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [IndexFG1]
GO
ALTER TABLE [dbo].[TeachersAndPupils]  WITH CHECK ADD  CONSTRAINT [FK_TeacherCharacters_TeachersAndPupils_TeacherCharacterID] FOREIGN KEY([TeacherCharacterID])
REFERENCES [dbo].[TeacherCharacters] ([TeacherCharacterID])
GO
ALTER TABLE [dbo].[TeachersAndPupils] CHECK CONSTRAINT [FK_TeacherCharacters_TeachersAndPupils_TeacherCharacterID]
GO
